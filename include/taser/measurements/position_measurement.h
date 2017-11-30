//
// Created by hannes on 2017-11-29.
//

#ifndef TASERV2_POSITION_MEASUREMENT_H
#define TASERV2_POSITION_MEASUREMENT_H

#include <Eigen/Dense>

#include <iostream>
#include "trajectory_estimator.h"

namespace taser {
namespace measurements {

class PositionMeasurement {
  using Vector3 = Eigen::Vector3d;
 public:
  PositionMeasurement(double t, const Vector3 &p) : t(t), p(p) {};

  // Measurement data
  double t;
  Vector3 p;

 protected:

  // Residual struct for ceres-solver
  template<template<typename> typename TrajectoryModel>
  struct Residual {
    Residual(const PositionMeasurement &m) : measurement(m) {};

    template <typename T>
    bool operator()(T const* const* params, T* residual) const {
      auto trajectory = TrajectoryModel<T>::Unpack(params, meta);
      Eigen::Matrix<T,3,1> p = trajectory.position(T(measurement.t));
      Eigen::Map<Eigen::Matrix<T,3,1>> r(residual);
      r = p - measurement.p.cast<T>();
      return true;
    }

    const PositionMeasurement& measurement;
    typename TrajectoryModel<double>::Meta meta;
  }; // Residual;

  template<template<typename> typename TrajectoryModel>
  void AddToEstimator(taser::TrajectoryEstimator<TrajectoryModel>& estimator) {
    using ResidualImpl = Residual<TrajectoryModel>;
    auto residual = new ResidualImpl(*this);
    auto cost_function = new ceres::DynamicAutoDiffCostFunction<ResidualImpl>(residual);
    std::vector<double*> parameter_blocks;
    std::vector<size_t> parameter_sizes;

    // Add trajectory to problem
    estimator.trajectory()->AddToProblem(estimator.problem(), residual->meta, parameter_blocks, parameter_sizes);
    for (auto ndims : parameter_sizes) {
      cost_function->AddParameterBlock(ndims);
    }

    // Add measurement
    cost_function->SetNumResiduals(3);
    // If we had any measurement parameters to set, this would be the place

    // Give residual block to estimator problem
    estimator.problem().AddResidualBlock(cost_function, nullptr, parameter_blocks);
  }

  // TrajectoryEstimator must be a friend to access protected members
  template<template<typename> typename TrajectoryModel>
  friend class taser::TrajectoryEstimator;
};

class AnotherMeasurement {
  using Vector3 = Eigen::Vector3d;
 public:
  AnotherMeasurement(double t, const Vector3 &p) : t_(t), p_(p) {};

  void hello() {
    std::cout << "Hello from AnotherMeasurement!" << std::endl;
  }

 protected:
  double t_;
  Vector3 p_;
};

} // namespace measurements
} // namespace taser


#endif //TASERV2_POSITION_MEASUREMENT_H