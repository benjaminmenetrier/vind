#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import copy
import numpy as np
import pyatlas
from NumPyDDL95 import tendency, tendencyTL, tendencyAD

def step(params, geom, t, xxField):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Parameters
  nx = params["nx"]
  ny = params["ny"]

  # Get view
  xxView = pyatlas.ndarray_view(xxField)

  # Check horizontal dimension
  if xxView.shape[0] != nx*ny:
    print("inconsistent horizontal size")
    exit(1)

  # Number of levels
  nz = xxView.shape[1]

  # Reshape
  xx = xxView.transpose().reshape((nz, ny, nx), copy=False)

  # Geometry
  lon = pyatlas.ndarray_view(geom["lon"])
  lat = pyatlas.ndarray_view(geom["lat"])
  cMask = pyatlas.ndarray_view(geom["cMask"])
  yMask = pyatlas.ndarray_view(geom["yMask"])

  # First step
  xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t, xx)*0.5*dti

  # Second step
  xx += tendency(params, lon, lat, cMask, yMask, t+0.5*dt, xxTmp)*dti

def stepTL(params, geom, t, xxTrajField, dxField):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Parameters
  nx = params["nx"]
  ny = params["ny"]

  # Get views
  xxTrajView = pyatlas.ndarray_view(xxTrajField)
  dxView = pyatlas.ndarray_view(dxField)

  # Check horizontal dimension
  if dxView.shape[0] != nx*ny:
    print("inconsistent horizontal size")
    exit(1)

  # Number of levels
  nz = dxView.shape[1]

  # Reshape
  xxTraj = xxTrajView.transpose().reshape((nz, ny, nx), copy=False)
  dx = dxView.transpose().reshape((nz, ny, nx), copy=False)

  # Geometry
  lon = pyatlas.ndarray_view(geom["lon"])
  lat = pyatlas.ndarray_view(geom["lat"])
  cMask = pyatlas.ndarray_view(geom["cMask"])
  yMask = pyatlas.ndarray_view(geom["yMask"])

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, cMask, yMask, t, xxTraj)*0.5*dti

  # First step
  dxTmp = dx+tendencyTL(params, cMask, yMask, xxTraj, dx)*0.5*dti

  # Second step
  dx += tendencyTL(params, cMask, yMask, xxTrajTmp, dxTmp)*dti

def stepAD(params, geom, t, xxTrajField, dxField):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Parameters
  nx = params["nx"]
  ny = params["ny"]

  # Get views
  xxTrajView = pyatlas.ndarray_view(xxTrajField)
  dxView = pyatlas.ndarray_view(dxField)

  # Check horizontal dimension
  if dxView.shape[0] != nx*ny:
    print("inconsistent horizontal size")
    exit(1)

  # Number of levels
  nz = dxView.shape[1]

  # Reshape
  xxTraj = xxTrajView.transpose().reshape((nz, ny, nx), copy=False)
  dx = dxView.transpose().reshape((nz, ny, nx), copy=False)

  # Geometry
  lon = pyatlas.ndarray_view(geom["lon"])
  lat = pyatlas.ndarray_view(geom["lat"])
  cMask = pyatlas.ndarray_view(geom["cMask"])
  yMask = pyatlas.ndarray_view(geom["yMask"])

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, cMask, yMask, t, xxTraj)*0.5*dti

  # Second step
  dxTmp = tendencyAD(params, cMask, yMask, xxTrajTmp, dx*dti)
  dx += dxTmp

  # First step
  dx += tendencyAD(params, cMask, yMask, xxTraj, dxTmp*0.5*dti)
