#include <MultiStepper.h>
#include <AccelStepper.h>
#include <StackArray.h>
// Button Pins
const int advanceButtonPin = 6;
const int retractButtonPin = 7;
const int zeroButtonPin = 8;

//Stepper Pins
const int enablePin = 6;
const int stepPin = 11;
const int directionPin = 12;

int buttons[] = { advanceButtonPin, retractButtonPin, zeroButtonPin };
double currentPosition = 0;
double fingerCount = 1;
int desiredCutWidth = 10;
double bladeWidth = 3.175;
bool advanceToNextSpace = false;
int stepsPerMM = 100;
StackArray <double> moveHistory;
AccelStepper stepper(1, stepPin, directionPin);

void setup()
{
	registerButtonPins();
	stepper.setMaxSpeed(1000);
	stepper.setSpeed(100);
	stepper.setAcceleration(300);
}

void loop()
{
	int button = readButtonInputs();
	if (button != -1) {
		buttonAction(button);
	}
	stepper.run();
}

void registerButtonPins() {
	int i;
	for (i = 0; i < sizeof(buttons); i = i + 1) {
		pinMode(buttons[i], INPUT);
	}
}

int readButtonInputs() {
	int i;
	for (i = 0; i < sizeof(buttons); i = i + 1) {
		int val = digitalRead(buttons[i]);
		if (val == HIGH) {
			return buttons[i];
		}
	}
	return -1;
}

void buttonAction(int buttonId) {
	switch (buttonId) {
	case advanceButtonPin:
		advance();
		break;
	case retractButtonPin:
		retract();
		break;
	case zeroButtonPin:
		setZero();
		break;
	default:
		break;
	}
}

int getNextMoveDistance() {
	//is the current finger space completely cut?
	if (advanceToNextSpace) {
		advanceToNextSpace = false;
		fingerCount += 1;
		currentPosition += desiredCutWidth; // advance position to next space
		return desiredCutWidth;
	}

	//calculate how far to the end of the current finger space
	int distanceToEndOfFinger = desiredCutWidth - ((currentPosition * fingerCount) + bladeWidth);

	if (distanceToEndOfFinger <= bladeWidth) {	//if the next move is bigger than the blade width then make an adjustment 
		currentPosition += distanceToEndOfFinger; // to create the last cut of the series
		advanceToNextSpace = true; // set up advance past next finger
		return distanceToEndOfFinger;
	}
	else {
		return bladeWidth; // advance by the blade width within the finger space
	}
}

void advance()
{
	double moveInMM = getNextMoveDistance();
	moveHistory.push(moveInMM);
	doAMove(moveInMM);
}

void retract()
{
	if (!moveHistory.isEmpty()) {
		doAMove(-moveHistory.pop());
	}
}

void fullRetract()
{
	doAMove(-currentPosition);
}

void doAMove(double move) {
	stepper.runToNewPosition(stepsPerMM * move);
}

void setZero()
{
	currentPosition = 0;
}
