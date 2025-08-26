/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "oops/runs/Run.h"

#include "saber/oops/ErrorCovarianceToolbox.h"
#include "saber/oops/instantiateCovarFactory.h"

#include "vind/Traits.h"

int main(int argc,  char ** argv) {
  oops::Run run(argc, argv);
  saber::instantiateCovarFactory<vind::Traits>();
  saber::ErrorCovarianceToolbox<vind::Traits> ect;
  return run.execute(ect);
}
