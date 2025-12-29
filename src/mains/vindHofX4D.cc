/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "oops/runs/HofX4D.h"
#include "oops/runs/Run.h"

#include "ufo/instantiateObsFilterFactory.h"
#include "ufo/ObsTraits.h"

#include "vind/Traits.h"

int main(int argc,  char ** argv) {
  oops::Run run(argc, argv);
  ufo::instantiateObsFilterFactory();
  oops::HofX4D<vind::Traits, ufo::ObsTraits> h4;
  return run.execute(h4);
}
