#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <limits>

class ECE_UAV
{
private:

	double delta_t = 0.03;
	std::mutex m_mutex;
	double x;
	double y;
	double z;
 
	double m_Position[3] = {};
	double m_Velocity[3] = {};
	double v;
	double vx;
	double vy;
	double vz;

	double a;
	double ax;
	double ay;
	double az;

	double dToCtr;	//distance from UAV to the center of sphere

	const double m =1;
	double Fmax;
	const double G = 9.8;

	double F;
	double Fx;
	double Fy;
	double Fz;

	double Fs;
	double Fs_x;
	double Fs_y;
	double Fs_z;

	bool stopThrd;
	bool arriveSurface;
	bool setOrbit;

	std::thread myThrd;
	int threadID;

	int color;
	bool increasing;
	

public:
	
	void setPosition(double x, double y, double z);
	void setVelocity(double vx, double vy, double vz);
	double getXPosition();
	double getYPosition();
	double getZPosition();

	double getVx();
	double getVy();
	double getVz();

	double distanceOf(double x1, double y1, double z1, double x2, double y2, double z2);

	friend void updateLocation(ECE_UAV* pUAV);
	void setThreadID(int id);
	void startThread();
	void stopThread();
	
	void elasticCollision(std::vector<ECE_UAV*> uav, int index);
	
};