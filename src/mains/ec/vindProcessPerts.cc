/*
 * (C) Copyright 2024 Meteorologisk Institutt.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include <limits>

#include "oops/runs/Run.h"
#include "oops/util/Logger.h"

#include "saber/oops/ProcessPerts.h"

#include "vind/instantiateQuenchMatrices.h"
#include "vind/Logbook.h"
#include "vind/Traits.h"

int main(int argc,  char ** argv) {
  oops::Run run(argc, argv);
  vind::instantiateQuenchMatrices();
  saber::ProcessPerts<vind::Traits> pp;
  vind::Logbook::start();
  run.execute(pp);
  vind::Logbook::stop();
  return 0;
}
