/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "oops/runs/LocalEnsembleDA.h"
#include "oops/runs/Run.h"

#include "ufo/instantiateObsFilterFactory.h"
#include "ufo/ObsTraits.h"

#include "vind/instantiateObsLocFactory.h"
#include "vind/Traits.h"

int main(int argc,  char ** argv) {
  oops::Run run(argc, argv);
  ufo::instantiateObsFilterFactory();
  vind::instantiateObsLocFactory();
  oops::LocalEnsembleDA<vind::Traits, ufo::ObsTraits> letkf;
  return run.execute(letkf);
}
