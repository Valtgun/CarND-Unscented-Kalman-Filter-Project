
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include "Eigen/Dense"
#include "ukf.h"
#include "ground_truth_package.h"
#include "measurement_package.h"

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

void check_arguments(int argc, char* argv[]) {
  string usage_instructions = "Usage instructions: ";
  usage_instructions += argv[0];
  usage_instructions += " path/to/input.txt output.txt";

  bool has_valid_args = false;

  // make sure the user has provided input and output files
  if (argc == 1) {
    cerr << usage_instructions << endl;
  } else if (argc == 2) {
    cerr << "Please include an output file.\n" << usage_instructions << endl;
  } else if (argc == 3) {
    has_valid_args = true;
  } else if (argc > 3) {
    cerr << "Too many arguments.\n" << usage_instructions << endl;
  }

  if (!has_valid_args) {
    exit(EXIT_FAILURE);
  }
}

void check_files(ifstream& in_file, string& in_name,
                 ofstream& out_file, string& out_name) {
  if (!in_file.is_open()) {
    cerr << "Cannot open input file: " << in_name << endl;
    exit(EXIT_FAILURE);
  }

  if (!out_file.is_open()) {
    cerr << "Cannot open output file: " << out_name << endl;
    exit(EXIT_FAILURE);
  }
}


float process(string in_file_name_, string out_file_name_, bool tune, VectorXd params) {

  ifstream in_file_(in_file_name_.c_str(), ifstream::in);
  ofstream out_file_(out_file_name_.c_str(), ofstream::out);

  check_files(in_file_, in_file_name_, out_file_, out_file_name_);

  /**********************************************
   *  Set Measurements                          *
   **********************************************/

  vector<MeasurementPackage> measurement_pack_list;
  vector<GroundTruthPackage> gt_pack_list;

  string line;

  // prep the measurement packages (each line represents a measurement at a
  // timestamp)
  while (getline(in_file_, line)) {
    string sensor_type;
    MeasurementPackage meas_package;
    GroundTruthPackage gt_package;
    istringstream iss(line);
    long long timestamp;

    // reads first element from the current line
    iss >> sensor_type;

    if (sensor_type.compare("L") == 0) {
      // laser measurement

      // read measurements at this timestamp
      meas_package.sensor_type_ = MeasurementPackage::LASER;
      meas_package.raw_measurements_ = VectorXd(2);
      float px;
      float py;
      iss >> px;
      iss >> py;
      meas_package.raw_measurements_ << px, py;
      iss >> timestamp;
      meas_package.timestamp_ = timestamp;
      measurement_pack_list.push_back(meas_package);
    } else if (sensor_type.compare("R") == 0) {
      // radar measurement

      // read measurements at this timestamp
      meas_package.sensor_type_ = MeasurementPackage::RADAR;
      meas_package.raw_measurements_ = VectorXd(3);
      float ro;
      float phi;
      float ro_dot;
      iss >> ro;
      iss >> phi;
      iss >> ro_dot;
      meas_package.raw_measurements_ << ro, phi, ro_dot;
      iss >> timestamp;
      meas_package.timestamp_ = timestamp;
      measurement_pack_list.push_back(meas_package);
    }

      // read ground truth data to compare later
      float x_gt;
      float y_gt;
      float vx_gt;
      float vy_gt;
      iss >> x_gt;
      iss >> y_gt;
      iss >> vx_gt;
      iss >> vy_gt;
      gt_package.gt_values_ = VectorXd(4);
      gt_package.gt_values_ << x_gt, y_gt, vx_gt, vy_gt;
      gt_pack_list.push_back(gt_package);
  }

  // Create a UKF instance
  UKF ukf;
  if (tune) {
    ukf.std_a_ = params(0);
    ukf.std_yawdd_ = params(1);
  }

  // used to compute the RMSE later
  vector<VectorXd> estimations;
  vector<VectorXd> ground_truth;

  // start filtering from the second frame (the speed is unknown in the first
  // frame)

  size_t number_of_measurements = measurement_pack_list.size();

  // column names for output file
  out_file_ << "px" << "\t";
  out_file_ << "py" << "\t";
  out_file_ << "v" << "\t";
  out_file_ << "yaw_angle" << "\t";
  out_file_ << "yaw_rate" << "\t";
  out_file_ << "px_measured" << "\t";
  out_file_ << "py_measured" << "\t";
  out_file_ << "px_true" << "\t";
  out_file_ << "py_true" << "\t";
  out_file_ << "vx_true" << "\t";
  out_file_ << "vy_true" << "\t";
  out_file_ << "NIS" << "\n";


  for (size_t k = 0; k < number_of_measurements; ++k) {
    // Call the UKF-based fusion
    ukf.ProcessMeasurement(measurement_pack_list[k]);

    // output the estimation
    out_file_ << ukf.x_(0) << "\t"; // pos1 - est
    out_file_ << ukf.x_(1) << "\t"; // pos2 - est
    out_file_ << ukf.x_(2) << "\t"; // vel_abs -est
    out_file_ << ukf.x_(3) << "\t"; // yaw_angle -est
    out_file_ << ukf.x_(4) << "\t"; // yaw_rate -est

    // output the measurements
    if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::LASER) {
      // output the estimation

      // p1 - meas
      out_file_ << measurement_pack_list[k].raw_measurements_(0) << "\t";

      // p2 - meas
      out_file_ << measurement_pack_list[k].raw_measurements_(1) << "\t";
    } else if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::RADAR) {
      // output the estimation in the cartesian coordinates
      float ro = measurement_pack_list[k].raw_measurements_(0);
      float phi = measurement_pack_list[k].raw_measurements_(1);
      out_file_ << ro * cos(phi) << "\t"; // p1_meas
      out_file_ << ro * sin(phi) << "\t"; // p2_meas
    }

    // output the ground truth packages
    out_file_ << gt_pack_list[k].gt_values_(0) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(1) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(2) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(3) << "\t";

    // output the NIS values

    if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::LASER) {
      out_file_ << ukf.NIS_laser_ << "\n";
    } else if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::RADAR) {
      out_file_ << ukf.NIS_radar_ << "\n";
    }


    // convert ukf x vector to cartesian to compare to ground truth
    VectorXd ukf_x_cartesian_ = VectorXd(4);

    float x_estimate_ = ukf.x_(0);
    float y_estimate_ = ukf.x_(1);
    float vx_estimate_ = ukf.x_(2) * cos(ukf.x_(3));
    float vy_estimate_ = ukf.x_(2) * sin(ukf.x_(3));

    ukf_x_cartesian_ << x_estimate_, y_estimate_, vx_estimate_, vy_estimate_;

    estimations.push_back(ukf_x_cartesian_);
    ground_truth.push_back(gt_pack_list[k].gt_values_);

  }

  // compute the accuracy (RMSE)
  Tools tools;
  if (!tune) {
    cout << "Accuracy - RMSE:" << endl << tools.CalculateRMSE(estimations, ground_truth) << endl;
  }

  // close files
  if (out_file_.is_open()) {
    out_file_.close();
  }

  if (in_file_.is_open()) {
    in_file_.close();
  }
  if (!tune) {
    cout << "Done!" << endl;
  }
  if (tune) {
    Eigen::VectorXd res = tools.CalculateRMSE(estimations, ground_truth);
    float score = res[0]*5+res[1]*5+res[2]+res[3];
    return score;
  }
  else {
    return 0.0;
  }
}

int main(int argc, char* argv[]) {
  // ********************************************************
  // adding function for gradient descent on parameter tuning
  // Twiddle from AI for Robotics course
  // ********************************************************

  // turn on of off the tuning
  // if the tuning is turned on, both files are calculated
  bool tune = false;

  check_arguments(argc, argv);
  float best_err = 0.0;
  string in_file_name_ = argv[1];
  string out_file_name_ = argv[2];

  VectorXd params = VectorXd(2);
  if (tune) {
    string in_file1 = "../data/sample-laser-radar-measurement-data-1.txt";
    string in_file2 = "../data/sample-laser-radar-measurement-data-2.txt";
    float stda = 2;
    float stdyawd = 1;
    params << stda, stdyawd;

    VectorXd d_params = VectorXd(2);
    d_params << 0.2,0.2;
    float tol = 0.001;
    //best_err = process(in_file2, out_file_name_, tune, params);
    best_err = process(in_file1, out_file_name_, tune, params) + process(in_file2, out_file_name_, tune, params);
    int epochs = 0;
    while (d_params.sum()>tol) {
      cout << "Epoch:" << epochs << endl;
      cout << "Dparam:" << d_params.sum() << endl;
      cout << "Params stda:" << params(0) << endl;
      cout << "Params stdyawd:" << params(1) << endl;
      cout << "Best err:" << best_err << endl;
      epochs++;
      for (int i=0; i<params.size(); i++) {
        params(i) += d_params(i);
        //int err = process(in_file2, out_file_name_, tune, params);
        float err = process(in_file1, out_file_name_, tune, params) + process(in_file2, out_file_name_, tune, params);
        cout << "err:" << err << endl;
        if (err < best_err)
        {
          best_err = err;
          d_params(i) *= 1.2;
        }
        else {
          params(i) -= 2*d_params(i);
          //int err = process(in_file2, out_file_name_, tune, params);
          float err = process(in_file1, out_file_name_, tune, params) + process(in_file2, out_file_name_, tune, params);
          cout << "err:" << err << endl;
          if (err < best_err)
          {
            best_err = err;
            d_params(i) *= 1.2;
          }
          else {
            params(i) += d_params(i);
            d_params(i) *= 0.9;
          }
        }
      }
    }
    cout << "Parameters:" << params << endl;
  }
  // ********************************************************
  // END of Twiddle
  // ********************************************************
  else {
    best_err = process(in_file_name_, out_file_name_, tune, params);
  }
  return best_err;
}
