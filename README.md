# Ozone-generator

This is a project developed in 2020 that consists of a Ozone Generator capable of outputing 20g/h of ozone. This product is used to eliminate odors inside a room or a vehicle.

The generator works on cycles in which the generator is on or off. The boost feature is represented by the generators working non-stop.

Program description

    boostPin - pin to boost push-button, low means active (because it is pulled up later in the setup func)
    startPin - pin to start push-button


Runtime Variables

    systemActive - mode of the sytem (0-off, 1-on)
    systemActiveID - id of the cycle time ----------------------------more on this later
    boostActive - boost button state (active or not)

    totalTime - Selected time of the generator to be on by potentiometer
    boostTIme - running time of the boost function

    cycleTime - counter of the cycle time
    onTime - selected time for generator to be on (in cycles) ----------------???????
    totalRunTime - total time for generator on
    boostMaxTime - time for boost to be on (maximum time) (in secounds)

Timers

    sysTimer1 - blocking/unblocking buttons
    sysTimer2 - duty cycle

    boostBlocked & startBlocked - variables to block the attempt of starting again a cycle

Setup Function 

    Serial is initialized on baudrate 9600
    boostPin & startPin are initialized as input_pullup pins = value of 0 means pressed
    Debounce objects are attached to both buttons (interval 5ms)

    Led pins, Fan pins and O3 gen pins are initalized as OUTPUTS
    Fan pins & gen pins are set to high ------------------------------------?????

Loop Function

    First we update de times and debounce objects

    Check if buttons have been pressed 
        
        If the boostBlocked is false and the boost button has been pressed then call boostUnboostSystem

        If startBlocked is false and the start buttons has been pressed then call startStopSystem

    Check if boost has to be disabled

        If boostActive is true and systemActive is true and boostTime is bigger than the maximum value of boost, then call boostUnboostSystem

BoostUnboostSystem Function

    Prevent pushing the boost button after the call of this function for 500ms 

    If boostActive is False then make it true and set boostTime to zero (also the LED)
    Else set boostActive to false and deactivate the led

startStopSystem

     Prevent pushing the start button after the call of this function for 500ms

     Reset boost timer to zero

     If the system is not active start the system
        Call readAnalogButtons - get the selected on time of the generators
                               - get the total time of the generator programme
        
        Set systemActive to true

        Call fanOn - this starts only the fans

        systemActiveID - calls dutycycle func every 1000ms - it is accomplished by a timer

    Else if the system is already running

        Set systemActive to false
        Turn off the O3 generators, and the fans
        sysTimer2.stop - stops the call of dutycycle function


DutyCycle Function

    First increments the timer with one

    If boost is active, increments the boost timer with one and turns on the generators

    If cycletime is bigger than the one selected turns on or off the generators

    Increase the cycle time at every call

    Reset cycleTime if it is bigger than 20