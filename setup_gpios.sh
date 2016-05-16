#!/bin/bash

# Example script of setting up GPIO ports

# list and exit
if [ "$1" == "list" ]; then
	ACTIVE=$(ls /sys/class/gpio/ | grep -E "gpio[0-9]")
	# echo "Active: $ACTIVE"
	
	for G in $ACTIVE; do
		echo -n "$G "
		cat "/sys/class/gpio/$G/direction"
	done

	if [ "$ACTIVE" == "" ]; then
		echo "No active GPIOs"
	fi
	exit
fi

# GPIO numbers used
TRIGGERS=(9 14 8 12)	# sensor triggers
ECHOES=(11 15 7 16)	# sensor echo
LEDS=(23 25)

ALLGPIOS=(${TRIGGERS[@]} ${ECHOES[@]} ${LEDS[@]})
OUTGPIOS=(${TRIGGERS[@]} ${LEDS[@]})
INGPIOS=(${ECHOES[@]})

if [ "$USER" != "root" ]; then
	echo
	echo "ERROR: must run as root" >&2
	exit 1
fi

echo "Setting up DriVR GPIO pins"
echo "Sensor triggers GPIOs: ${TRIGGERS[@]}"
echo "Sensor echo GPIOs: ${TRIGGERS[@]}"
echo "LED GPIOs: ${TRIGGERS[@]}"

# unexport
for GPIO in ${ALLGPIOS[@]}; do 
	echo unexport $GPIO
	echo $GPIO > /sys/class/gpio/unexport 2> /dev/null 
done

# Only unexport
if [ "$1" == "unexport" ]; then
	exit
fi

# export
sleep 0.1
for GPIO in ${ALLGPIOS[@]}; do 
	echo export $GPIO
	echo $GPIO > /sys/class/gpio/export 
	chmod -R a+rw /sys/class/gpio/gpio$GPIO/
done
sleep 0.1

# triggers (outputs)
for GPIO in ${OUTGPIOS[@]}; do 
	echo "out $GPIO (trigger)"
	echo out > /sys/class/gpio/gpio$GPIO/direction
done

# echoes (inputs)
for GPIO in ${INGPIOS[@]}; do 
	echo "in $GPIO (echo)"
	echo in > /sys/class/gpio/gpio$GPIO/direction
done
