/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include <limits>

#include "oops/runs/AssimEnsemblePatched.h"
#include "oops/runs/Run.h"
#include "oops/util/Logger.h"

#include "saber/oops/instantiateCovarFactory.h"

#include "vind/instantiateQuenchMatrices.h"
#include "vind/Logbook.h"
#include "vind/Traits.h"

int main(int argc, char** argv) {
  oops::Run run(argc, argv);
  oops::Log::test().setf(std::ios::scientific);
  oops::Log::test().precision(std::numeric_limits<double>::digits10+1);
  vind::instantiateQuenchMatrices();
  saber::instantiateCovarFactory<vind::Traits>();
  oops::AssimEnsemblePatched<vind::Traits> aep;
  vind::Logbook::start();
  run.execute(aep);
  vind::Logbook::stop();
  return 0;
}
