#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

unsigned long long ta_davaj_cas() {
	struct timeval tv;
	gettimeofday(&tv,NULL);	
	return 1000000 * (unsigned long long) tv.tv_sec + tv.tv_usec;
}

int ta_otvor(int ta_gpio, int ta_vlajky) {
	char ta_buf[200];
	sprintf(ta_buf, "/sys/class/gpio/gpio%d/value", ta_gpio);

	int ta_fajl = open (ta_buf, ta_vlajky);
	if(ta_fajl < 0 ) {
		fprintf(stderr, "NASRAC: Ta vyrucilo sa pri otvarani %s\n", ta_buf);
		exit(1);
	}
	return ta_fajl;	
}

//int ta_citaj(FILE *f) {
int ta_citaj(int ta_fajl) {
	char abo_daco;
	lseek(ta_fajl, 0, SEEK_SET);
	if(read(ta_fajl, &abo_daco, 1) != 1) {
		fprintf(stderr, "NASRAC: neprecitalo banany\n");
		exit(1);
	}
	return abo_daco - '0';
}

void ta_zapis(int ta_fajl, int ta_daco) {
	char abo_daco = '0' + ta_daco;
	if (write(ta_fajl, &abo_daco, 1) != 1) {
		fprintf(stderr, "NASRAC: nezapisalo banany\n");
		exit(1);
	}
} 

unsigned long long  timedifference(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000000ULL + (t1.tv_usec - t0.tv_usec);
}

typedef enum sensor_state {
	ZERO,
	MEASURE,
	DONE
} sensor_state;

int ta_su_us_sicke(sensor_state *states, int n) {
	int i;
	for (i = 0; i < n; i++) {
		if (states[i] != DONE)
			return 0;
	}
	return 1; //Vyborne, gol
}

unsigned long long ta_meraj_jeden(int ftrigger, int fecho) {
	struct timeval start, end;

	ta_zapis(ftrigger, 1);
	usleep(10);
	ta_zapis(ftrigger, 0);
	
	while (ta_citaj(fecho) ==  0)
		;
	gettimeofday(&start, NULL);

	while (ta_citaj(fecho) ==  1)
		;
	gettimeofday(&end, NULL);

	return timedifference(start, end);
} 

void ta_meraj_sicke(int *trig, int *echo, unsigned long long **durations, int sensor_count, int repetitions, int settleup_us) {
	int i, sen;
	
	for (i = 0; i < repetitions; i++) {
		// Settle up
		for (sen = 0; sen < sensor_count; sen++)
			ta_zapis(trig[sen], 0);
		usleep(settleup_us);		

		// Measure	
		for (sen = 0; sen < sensor_count; sen++) {
			//printf("i=%d sen=%d\n", i, sen);
			durations[sen][i] = ta_meraj_jeden(trig[sen], echo[sen]);
		}
	}
}

// Used for qsort
int ta_porovnaj(const void *a, const void *b) {
	return ( *(unsigned long long*)a - *(unsigned long long*)b );   
}


// Sort measurement for each sensor
void ta_utried_merania(unsigned long long **durations, int sensor_count, int repetitions) {
	int i;

	for (i = 0; i < sensor_count; i++) {
		qsort(durations[i], repetitions, sizeof(unsigned long long), ta_porovnaj);
	} 
}

void ta_vyruc_na_obrazovku(unsigned long long **durations, int sensor_count, int repetitions, int debug) {
	int i;
	int ta_stred = repetitions / 2;
	//printf("Toto nemala robic ta_stred=%d cnt=%d rep=%d\n", ta_stred, sensor_count, repetitions);
	for (i = 0; i < sensor_count; i++) {
		unsigned long long duration = durations[i][ta_stred];

		unsigned long long distance = duration * 1715 / 100000; 
		printf("%llu ", distance);
		if(debug) {
			printf("(%llu) ", duration);
		
			int j;
			printf("-> [");
			for(j = 0; j < repetitions; j++) {
				unsigned long long dis = durations[i][j] * 1715 / 100000;
				printf("%llu", dis);
				if (j != repetitions - 1)
					putchar(',');
			}
			printf("]\n");
		}
	}
	putchar('\n');
}

int main(int argc, char **argv) {
	int i;
	int debug = 0;
	int settleup = 1000;
	int repetitions = 5;

	if (argc < 3) {
		fprintf(stderr, "usage: %s [TRIGGER_GPIO] [ECHO_GPIO]... [debug]\n", argv[0]);
		exit(1);
	}
	if (strcmp(argv[argc - 1], "debug") == 0) 
		debug = 1;
	
	int sensor_count = (argc - 1) / 2;

	int *trig = malloc(sensor_count * sizeof(int));
	int *echo   = malloc(sensor_count * sizeof(int));

	for (i = 0; i < sensor_count; i++) {
		int gpio = atoi(argv[1 + 2 * i]);
		trig[i] = ta_otvor(gpio, O_WRONLY);

		gpio = atoi(argv[2 + 2 * i]);
		echo[i] = ta_otvor(gpio, O_RDONLY);
	}

	struct timeval *starts = malloc(sensor_count * sizeof(struct timeval));
	struct timeval *ends   = malloc(sensor_count * sizeof(struct timeval));
	
	unsigned long long **durations = malloc(sensor_count * sizeof(unsigned long long*));
	for (i = 0; i < sensor_count; i++) { 
		durations[i] = malloc(repetitions * sizeof(unsigned long long));	
		
	}

	ta_meraj_sicke(trig, echo, durations, sensor_count, repetitions, settleup); 
	ta_utried_merania(durations, sensor_count, repetitions);
	ta_vyruc_na_obrazovku(durations, sensor_count, repetitions, debug);
	
	// Clean up
	for (i = 0; i < sensor_count; i++) {
		close(trig[i]);
		close(echo[i]);
	}

	free(trig);	
	free(echo);
	free(starts);
	free(ends);

	return 0;
}
