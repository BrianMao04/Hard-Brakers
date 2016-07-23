//////////////////////////////////////
//The Hard Brakers Main Program
//Traxxas 5
//Jeffray Behr, Brian Mao, Macgregor Paddock, Mariko Shimoda
//////////////////////////////////////

//Prototypes
void wheel_speeds(float & speed_front, float & speed_rear);
void activate_brakes_turning(string direction);
void return_cam(int degrees, string direction);
void turning_detection(string & direction);
void spoiler_raiser(float speed_rear, bool &raised);
void activate_brakes_skidding(float speed_front, float speed_rear);
void emergency_brake(float &speed_front, float &speed_rear);

const float radius = 0.055;
string reset_motor_a = "Right", reset_motor_c = "Left";

//////////////////////////////////////
task main()
{
	//SensorType in S1 determined later
	SensorType[S2]= sensorTouch;
	SensorType[S3] = sensorColorNxtFULL;
	SensorType[S4] = sensorColorNxtFULL;

	float speed_front = 0.0, speed_rear = 0.0;
	string direction;
	bool raised = false, UltraHard = false;

	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorB] = 0;
	nMotorEncoder[motorC] = 0;

	displayString(0, "Press Right");
	displayString(1, "for Normal");
	displayString(3, "Press Left");
	displayString(4, "for UltraHard");
	displayString(6, "Switch");
	displayString(7, "input cords.");

	//Determine Which Mode the Program Will Activate (With or without Ultrasound Sensor)
	while (nNxtButtonPressed != 1 && nNxtButtonPressed != 2){}
	if (nNxtButtonPressed == 1)
	{
		SensorType[S1] = sensorTouch;
	}
	else if (nNxtButtonPressed == 2)
	{
		SensorType[S1] = sensorSONAR;
		UltraHard = true;
	}

	//Main Program Loop
	while (nNxtButtonPressed!=3)
	{
		wheel_speeds(speed_front, speed_rear);
		eraseDisplay();
		displayString(1, "%.2f", speed_front);
		displayString(2, "%.2f", speed_rear);
		turning_detection(direction);
		displayString(0, "%s", direction);
		activate_brakes_skidding(speed_front,speed_rear);
		spoiler_raiser(speed_rear, raised);

		if (UltraHard)
		{
			emergency_brake(speed_front, speed_rear);
		}

	}

	while (nNxtButtonPressed==3){}
}

/////////////////////////////////////////
void wheel_speeds(float & speed_front, float & speed_rear)
{
	int initial_front, initial_rear, count_front = 0, count_rear = 0;
	time1[0] = 0;
	while(time1[0] <= 500)
	{
		initial_front = SensorValue[S3];
		initial_rear = SensorValue[S4];
		wait1Msec(1);

		if (initial_front != SensorValue[S3])
		{
			count_front += 1;
			initial_front = SensorValue[S3];
		}

		if (initial_rear != SensorValue[S4])
		{
			count_rear += 1;
			initial_rear = SensorValue[S4];
		}

	}
	speed_front = 2 * PI * radius * count_front / 2 / 0.5;
	speed_rear = 2 * PI * radius * count_rear / 2 / 0.5;
}

//////////////////////////////////
void activate_brakes_turning(string direction)
{
	if (direction == "Right")
		motor[motorA] = -100;
	else if (direction == "Left")
		motor[motorC] = -100;
}

////////////////////////////////
void return_cam(int degrees, string direction)
{
	if (degrees != 0)
	{
		nMotorEncoder[motorA] = 0;
		nMotorEncoder[motorC] = 0;
		if (degrees <= 180)
		{
			if (direction == "Right")
			{
				while (nMotorEncoder[motorA] < degrees)
				{
					motor[motorA] = 30;
				}
				motor[motorA] = 0;
			}
			else if (direction == "Left")
			{
				while (nMotorEncoder[motorC] < degrees)
				{
					motor[motorC] = 30;
				}
				motor[motorC] = 0;
			}
		}
		else
		{
			if (direction == "Right")
			{
				while (nMotorEncoder[motorA] > -1 * (360 - degrees))
				{
					motor[motorA] = -30;
				}
				motor[motorA] = 0;
			}
			else if (direction == "Left")
			{
				while (nMotorEncoder[motorC] > -1 * (360 - degrees))
				{
					motor[motorC] = -30;
				}
				motor[motorC] = 0;
			}
		}
	}
}

//////////////////////////////////
void turning_detection(string & direction)
{
	int degrees;
	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorC] = 0;
	if (SensorValue[S1] == 1)
	{
		direction = "Right";
		while (SensorValue[S1] == 1)
		{
			activate_brakes_turning(direction);//Keep applying the brakes until car is  not turning anymore
			displayString(0, "%s", direction);
		}
		motor[motorA] = 0;
		degrees = abs(nMotorEncoder[motorA] % 360);
  	return_cam(degrees, direction); //Reset cam position once brakes no longer need to be applied
	}

	else if (SensorValue[S2] == 1)
	{
		direction = "Left";
		while (SensorValue[S2] == 1)
		{
			activate_brakes_turning(direction);
			displayString(0, "%s", direction);
		}
		motor[motorC] = 0;
		degrees = abs(nMotorEncoder[motorC] % 360);
  	return_cam(degrees, direction);
	}

	else
	{
		direction = "Not Turning";
	}

}

////////////////////////////////
void spoiler_raiser (float speed_rear, bool & raised)
{
	nMotorEncoder[motorB]=0;

	if (speed_rear>=2 && !raised) //Raise spoiler when the car is travelling faster than 2 m/s
	{
		while (nMotorEncoder[motorB]>=-45)
		{
			motor[motorB]=-10;
		}
		raised=true;
		displayString(4, "Spoiler Raised");
	}

	else if (speed_rear<2&&raised)
	{
		while (nMotorEncoder[motorB]<=40)
		{
			motor[motorB]=10; //Lower the spoiler back down once car has slowed down
		}
		raised=false;
		displayString(4, "Not Raised");
	}
	motor[motorB]=0;
}

/////////////////////////////////////////
void activate_brakes_skidding(float speed_front, float speed_rear)
{
	const float tolerance = 1.5; //Tolerance used to measure the difference in wheel speeds
	int degreesA=0;
	int degreesC=0;

	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorC] = 0;

	while (abs(speed_front-speed_rear)>tolerance)
	{
		motor[motorA] = motor[motorC] = -100; //Apply brakes when difference in wheel speeds are too great
		wheel_speeds(speed_front, speed_rear);
	}

	motor[motorA] = motor[motorC] = 0;

	degreesA = abs(nMotorEncoder[motorA] % 360);
	degreesC = abs(nMotorEncoder[motorC] % 360);

	return_cam(degreesA, reset_motor_a); //Reset cam position once brakes no longer need to be applied
	return_cam(degreesC, reset_motor_c);
}

/////////////////////////////////////////
void emergency_brake(float &speed_front, float &speed_rear)
{
	float ultrasound_values[10]= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	float ultrasound_sum = 0, ultrasound_avg=0;
	int values_to_count = 1;
	int degreesA=0,degreesC=0;
	string reset_motor_a = "Right";
	string reset_motor_c = "Left";

	for (int i=0; i<10; i++) //Gather a set of 10 readings from sensor to improve accuracy
	{
		if (SensorValue[S1]<255) //Eliminates the "255" measurements from bad readings
			{
			ultrasound_values[i]=SensorValue[S1];
			values_to_count+=1;
			wait1Msec(10); //Small delay to avoid taking in the exact same reading
			}

		ultrasound_sum+=ultrasound_values[i];
	}

	ultrasound_avg = ultrasound_sum/values_to_count;
	displayString(7,"%f",ultrasound_avg);


	if (ultrasound_avg <= 200 && ultrasound_avg>0.1)
	{
		motor[motorA] = -100; //Activate the brakes if the car is within 1m of an oncoming obstacle
		motor[motorC] = -100;

		displayString (6,"Emergency Stop!");

		while (speed_rear > 1)
			{
			wheel_speeds(speed_front,speed_rear); //Continue to apply brakes until the car stops
			}

		motor[motorA] = 0; //Stop appying after car has stopped
		motor[motorC] = 0;

		degreesA = abs(nMotorEncoder[motorA] % 360);
		degreesC = abs(nMotorEncoder[motorC] % 360);
	}
	//---------------------

	ultrasound_sum = 0; //Reset values for the next set of 10 measurements
	values_to_count = 0;

	if (ultrasound_avg<100) //Reset cam position once brakes no longer need to be applied
	{
		return_cam(degreesA, reset_motor_a);
		return_cam(degreesC, reset_motor_c);
	}

}

//////////////////////////////////
