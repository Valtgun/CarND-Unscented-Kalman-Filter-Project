#ifndef TOOLS_H_
#define TOOLS_H_
#include <vector>
#include "Eigen/Dense"

class Tools {
public:
  /**
  * Constructor.
  */
  Tools();

  /**
  * Destructor.
  */
  virtual ~Tools();

  /**
  * A helper method to calculate RMSE.
  */
  Eigen::VectorXd CalculateRMSE(const std::vector<Eigen::VectorXd> &estimations, const std::vector<Eigen::VectorXd> &ground_truth);

  // Helper function to convert from Cartesian To Polar coordinates
  Eigen::VectorXd CartesianToPolar(const Eigen::VectorXd &x_state);

  // Helper function to convert from Polar To Cartesian coordinates
  Eigen::VectorXd PolarToCartesian(const Eigen::VectorXd &z);

};

#endif /* TOOLS_H_ */
