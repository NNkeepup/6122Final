/*
Author: <Nuo Wang>
Class: ECE6122
Last Date Modified: <12/07/2021>
Description:
Source file to create ECE_UAV class and star/end thread, and update kinematic information.
Reference:Some code that professor did in the class, mainly about how to generate force.
*/

#include"ECE_UAV.h"
#include <sys/timeb.h>
#include <iostream>


void ECE_UAV::setPosition(double x, double y, double z)
{
	//Hold postions
	m_mutex.lock();
	this->m_Position[0] = x;
	this->m_Position[1] = y;
	this->m_Position[2] = z;
	m_mutex.unlock();
}
void ECE_UAV::setVelocity(double vx, double vy, double vz)
{
	//Hold velocities
	m_mutex.lock();
	this->m_Velocity[0] = vx;
	this->m_Velocity[1] = vy;
	this->m_Velocity[2] = vz;
	m_mutex.unlock();
}
void ECE_UAV::setThreadID(int id)
{
	//set thread ID
	threadID = id;
}

double ECE_UAV::getXPosition()
{
	return this->m_Position[0];
}

double ECE_UAV::getYPosition()
{
	return this->m_Position[1];
}

double ECE_UAV::getZPosition()
{
	return this->m_Position[2];
}

double ECE_UAV::getVx()
{
	return this->m_Velocity[0];
}

double ECE_UAV::getVy()
{
	return this->m_Velocity[1];
}

double ECE_UAV::getVz()
{
	return this->m_Velocity[2];
}


double ECE_UAV::distanceOf(double x1, double y1, double z1, double x2, double y2, double z2)
{
	//calculate distance of two vector
	double distance;
	distance = sqrt(pow(x1-x2,2)+pow(y1-y2,2)+pow(z1-z2,2));
	return distance;
}


void updateLocation(ECE_UAV* pUAV)
{
	//Initialization of uavs' kinematic information
	pUAV->vx = 0;
	pUAV->vy = 0;
	pUAV->vz = 0;
	pUAV->v = 0;
	pUAV->a =0, pUAV->ax=0, pUAV->ay=0, pUAV->az = 0;
	pUAV->F=0, pUAV->Fx =0, pUAV->Fy=0, pUAV->Fz = 0;
	pUAV->Fs=0, pUAV->Fs_x=0, pUAV->Fs_y=0, pUAV->Fs_z = 0;
	pUAV->increasing = false;
	double F1 = 0;
	double F2 = 0;
	double F3 = 0;

	////Initialization of thread information
	pUAV->stopThrd = false;
	pUAV->arriveSurface = false;
	pUAV->stopThrd = false;
	
	//First uav has to stay on the ground for 5 secs
	std::this_thread::sleep_for(std::chrono::seconds(5));

	//flag helps to generate random numbers 
	int countFlyTime = 0;
	bool randomFlag1 = 0;
	bool randomFlag2 = 0;

	//keep update the uavs' kinematic information untill main function stops thread
	do
	{
		
		
		//get position and velocity
		pUAV->x = pUAV->getXPosition();
		pUAV->y = pUAV->getYPosition();
		pUAV->z = pUAV->getZPosition();

		pUAV->vx = pUAV->getVx();
		pUAV->vy = pUAV->getVy();
		pUAV->vz = pUAV->getVz();
		
		//update a while moving towards center
		pUAV->ax = -10.0 * pUAV->x / pUAV->dToCtr;
		pUAV->ay = -10.0 * pUAV->y / pUAV->dToCtr;
		pUAV->az = -10.0 * (pUAV->z - 50) / pUAV->dToCtr + pUAV->G / pUAV->m;		// the additional G is gravity

		pUAV->x += pUAV->vx * pUAV->delta_t + 0.5 * pow(pUAV->delta_t, 2) * pUAV->ax;// v*t + 1/2*a*(t^2)
		pUAV->y += pUAV->vy * pUAV->delta_t + 0.5 * pow(pUAV->delta_t, 2) * pUAV->ay;
		pUAV->z += pUAV->vz * pUAV->delta_t + 0.5 * pow(pUAV->delta_t, 2) * (pUAV->az - pUAV->G / pUAV->m);
		
		pUAV->dToCtr = pUAV->distanceOf(pUAV->x, pUAV->y, pUAV->z, 0, 0, 50);  // update distance

		if (!pUAV->arriveSurface && pUAV->v >= 2.0)
		{
			//Velocity exceeds limits when approaching sphere
			pUAV->ax = 0.0;
			pUAV->ay = 0.0;
			pUAV->az = pUAV->G / pUAV->m;
		}

		if (!pUAV->arriveSurface && pUAV->dToCtr <= 12)
		{
			//slowing down when approaching sphere
			pUAV->ax = pUAV->x / pUAV->dToCtr;
			pUAV->ay = pUAV->y / pUAV->dToCtr;
			pUAV->az = (pUAV->z - 50.0) / pUAV->dToCtr + 10.0;
		}

		if (pUAV->dToCtr <= 10)
		{
			//set a flag to show uav has reached the surface
			pUAV->arriveSurface = true;
		}

		if (pUAV->arriveSurface)
		{
			//generate a tangent force
			if (randomFlag2 == 0)
			{
				pUAV->vx = 0;
				pUAV->vy = 0;
				pUAV->vz = 0;
				struct timeb timeSeed;
				ftime(&timeSeed);
				srand(timeSeed.time * 1000 + timeSeed.millitm + pUAV->threadID);  // millisecond
				
				do {
					F1 = (double)(rand() % 20) - 10;	//generate a random number from [-10,10]
					F2 = (double)(rand() % 20) - 10;
				} while (F1 * F2 == 0.0);

				randomFlag2 = 1;

			}
			
			pUAV->Fz = (-F1 * pUAV->x - F2 * pUAV->y) / (pUAV->z - 50.0);
			pUAV->Fx = F1;
			pUAV->Fy = F2;
			
			//Normalization
			pUAV->F = pUAV->distanceOf(pUAV->Fx, pUAV->Fy, pUAV->Fz, 0, 0, 0);
			pUAV->Fx /= pUAV->F;
			pUAV->Fy /= pUAV->F;
			pUAV->Fz /= pUAV->F;

			// Generate Fs based on Hooke's Law
			pUAV->Fs = 1.0 * (pUAV->dToCtr - 10.0);
			pUAV->Fs_x = -pUAV->x * pUAV->Fs / pUAV->dToCtr; 
			pUAV->Fs_y = -pUAV->y * pUAV->Fs / pUAV->dToCtr;
			pUAV->Fs_z = -(pUAV->z - 50) * pUAV->Fs / pUAV->dToCtr + 10.0;

			pUAV->v = pUAV->distanceOf(pUAV->vx, pUAV->vy, pUAV->vz, 0, 0, 0);

			pUAV->ax = pUAV->Fs_x;
			pUAV->ay = pUAV->Fs_y;
			pUAV->az = pUAV->Fs_z;
		
			int rand_num = 0;
			if (randomFlag1 == 0)
			{
				//only need to generate this random number once
				rand_num = rand() % 2;  //0 or 1;
				randomFlag1 = 1;
			}
			countFlyTime++;

			if (countFlyTime <= 5)
			{
				//generate a random force direction
				if (rand_num == 1)
				{
					pUAV->ax += pUAV->Fx;
					pUAV->ay += pUAV->Fy;
					pUAV->az += pUAV->Fz;
				}
				else
				{
					pUAV->ax -= pUAV->Fx;
					pUAV->ay -= pUAV->Fy;
					pUAV->az -= pUAV->Fz;
				}
				
			}
			else
			{
				//curce the speed to maintain a constant velocity
				if (pUAV->v <=5)
				{
					pUAV->ax += pUAV->vx / pUAV->v ;
					pUAV->ay += pUAV->vy / pUAV->v ;
					pUAV->az += pUAV->vz / pUAV->v;
					
				}
				if (pUAV->v >5 )
				{
					pUAV->ax -= pUAV->vx / pUAV->v;
					pUAV->ay -= pUAV->vy / pUAV->v;
					pUAV->az -= pUAV->vz / pUAV->v;
				}
			}
		}
		//Calculate and set velocity 
		pUAV->vx += pUAV->ax * pUAV->delta_t;
		pUAV->vy += pUAV->ay * pUAV->delta_t;
		pUAV->vz += (pUAV->az - pUAV->G/ pUAV->m) * pUAV->delta_t;
		pUAV->v = pUAV->distanceOf(pUAV->vx, pUAV->vy, pUAV->vz,0,0,0);

		//update velocity and position before go into next loop turn
		pUAV->setVelocity(pUAV->vx, pUAV->vy, pUAV->vz);
		pUAV->setPosition(pUAV->x, pUAV->y, pUAV->z);
		
		//updates the kinematic information every 10 msec
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
	while (!pUAV->stopThrd);
}

void ECE_UAV::startThread()
{
	myThrd= std::thread(updateLocation,this);

}

void ECE_UAV::stopThread()
{
	stopThrd = true;
	if (myThrd.joinable())
	{
		myThrd.join();
	}
}

void ECE_UAV::elasticCollision(std::vector<ECE_UAV*> uavs, int index)
{
	//detect elastic collision
	//pull uavs off a certain distance to get out of the current collision state,
	//otherwise the 2 uavs will keep exchanging velocity and get stuck in loop
	x = getXPosition();
	y = getYPosition();
	z = getZPosition();
	
	int closestUAV = -1;
	double dLimit = std::numeric_limits<double>::max();
	for (int i = 0;i < 15; i++)
	{
		if (i != index)
		{
			double dToOtherUAV = distanceOf(uavs[i]->getXPosition(), uavs[i]->getYPosition(), uavs[i]->getZPosition(), x, y, z);

			if (dToOtherUAV <= 1 && dToOtherUAV <= dLimit  )
			{
				//find a closest uav make sure collision will happen only between 2 uavs at a time 
				closestUAV = i;
				dLimit = dToOtherUAV;
			}
		}
	}
	if (closestUAV != -1)
	{
		//elastic collision happens 
		//2 uavs exchange velocity and escape from each other a certain distance

		//exchange velocity
		double tempVx = vx;
		double tempVy = vy;
		double tempVz = vz;

		vx = uavs[closestUAV]->getVx();
		vy = uavs[closestUAV]->getVy();
		vy = uavs[closestUAV]->getVz();

		uavs[closestUAV]->setVelocity(tempVx, tempVy, tempVz);

		setVelocity(vx, vy, vz);

		//escape from each other a certain distance
		double x_mid = (x + uavs[closestUAV]->getXPosition()) / 2;
		double y_mid = (y + uavs[closestUAV]->getYPosition()) / 2;
		double z_mid = (z + uavs[closestUAV]->getZPosition()) / 2;

		double d_mid = distanceOf(this->x, this->y, this->z, x_mid,y_mid,z_mid);

		this->x = x_mid + (this->x - x_mid) / d_mid * 1.0;
		this->y = y_mid + (this->y - y_mid) / d_mid * 1.0;
		this->z = z_mid + (this->z - z_mid) / d_mid * 1.0;

		setPosition(this->x, this->y, this->z);

		uavs[closestUAV]->setPosition(x_mid + (uavs[closestUAV]->getXPosition() - x_mid) / d_mid * 1.0,
			y_mid + (uavs[closestUAV]->getYPosition() - y_mid) / d_mid * 1.0,
			z_mid + (uavs[closestUAV]->getZPosition() - z_mid) / d_mid * 1.0);



	}
}


