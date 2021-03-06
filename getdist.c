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

int ta_otvor(int ta_gpio, int ta_flagy) {
	char ta_buf[200];
	sprintf(ta_buf, "/sys/class/gpio/gpio%d/value", ta_gpio);

	int ta_fajl = open (ta_buf, ta_flagy);
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
	return 1;
}

int main(int argc, char **argv) {
	int i;
	int debug = 0;

	if (argc < 3) {
		fprintf(stderr, "usage: %s [TRIGGER_GPIO] [ECHO_GPIO]\n", argv[0]);
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
	sensor_state *states = malloc(sensor_count * sizeof(sensor_state));
	for (i = 0; i < sensor_count; i++)
		states[i] = ZERO;

	// Settle up
	for (i = 0; i < sensor_count; i++)
		ta_zapis(trig[i], 0);
	usleep(200000);	
	
	// Trigger
	for (i = 0; i < sensor_count; i++)
		ta_zapis(trig[i], 1);

	usleep(10);
	
	for (i = 0; i < sensor_count; i++)
		ta_zapis(trig[i], 0);


	// Echo
	do {
		for (i = 0; i < sensor_count; i++) {
			if (states[i] == ZERO) {
				if (ta_citaj(echo[i]) == 1) {
					gettimeofday(&starts[i], NULL);
					states[i] = MEASURE;
				}
			} else if (states[i] == MEASURE) {
				if (ta_citaj(echo[i]) == 0) {
					gettimeofday(&ends[i], NULL);
					states[i] = DONE;
				}
			}
		}
	} while (!ta_su_us_sicke(states, sensor_count));

	// Output
	for (i = 0; i < sensor_count; i++) {
		unsigned long long duration = timedifference(starts[i], ends[i]);

		unsigned long long distance = duration * 1715 / 100000; 
		//printf("begin %llu end %llu duration %llu\n", start, end, duration);
		printf("%llu", distance);
		if(debug)
			printf("(%llu) ", duration);
		else
			putchar(' ');
	}
	putc('\n', stdout);

	// Clean up
	for (i = 0; i < sensor_count; i++) {
		close(trig[i]);
		close(echo[i]);
	}

	free(trig);	
	free(echo);
	free(starts);
	free(ends);
	free(states);

	return 0;
}
