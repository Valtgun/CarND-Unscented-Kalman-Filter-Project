#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  VectorXd rmse(4);
  rmse << 0,0,0,0;
  // check the validity of the following inputs:
  //  * the estimation vector size should not be zero
  //  * the estimation vector size should equal ground truth vector size
  if(estimations.size() != ground_truth.size() || estimations.size() == 0){
    std::cout << "Invalid estimation or ground_truth data" << std::endl;
    return rmse;
  }

	//accumulate squared residuals
	for(unsigned int i=0; i < estimations.size(); ++i){
    VectorXd residual = estimations[i] - ground_truth[i];
		//coefficient-wise multiplication
		residual = residual.array()*residual.array();
		rmse += residual;
	}

	//calculate the mean
	rmse = rmse/estimations.size();

	//calculate the squared root
	rmse = rmse.array().sqrt();

	//return the result
	return rmse;
}

VectorXd Tools::CartesianToPolar(const VectorXd &x_state) {
  float px = x_state(0);
  float py = x_state(1);
  float vx = x_state(2);
  float vy = x_state(3);

  float rho = sqrt(px*px + py*py);
  float phi = atan2(py, px);
  while (phi < -M_PI)
    phi += M_PI;
  while (phi > M_PI)
    phi -= M_PI;
  float rho_dot = (px*vx + py*vy) / rho;

  VectorXd polar = VectorXd(3);
  polar << rho, phi, rho_dot;
  return polar;
}

VectorXd Tools::PolarToCartesian(const Eigen::VectorXd &z)
{
  float rho = z(0);
  float phi = z(1);
  float rho_dot = z(2);

  float py = rho * sin(phi);
  float px = rho * cos(phi);
  float vx = rho_dot * cos(phi);
  float vy = rho_dot * sin(phi);
  VectorXd cartesian(4);
  cartesian << px, py, vx, vy;
  return cartesian;
}
