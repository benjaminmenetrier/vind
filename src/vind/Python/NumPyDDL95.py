#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import numpy as np

def step(params, lon, lat, cMask, yMask, t, xx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # First step
  xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t, xx)*0.5*dti

  # Second step
  xx += tendency(params, lon, lat, cMask, yMask, t+0.5*dt, xxTmp)*dti

def tendency(params, lon, lat, cMask, yMask, t, xx):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  F = params["F"]
  omega = params["omega"]
  nu = params["nu"]

  # Number of levels
  nz = xx.shape[0]

  # Check horizontal dimensions
  if xx.shape[1] != ny:
    print("inconsistent ny dimension")
    exit(1)
  if xx.shape[2] != nx:
    print("inconsistent nx dimension")
    exit(1)

  # Create tendency
  xxTen = np.zeros((nz,ny,nx))

  # Time-dependent forcing
  FF = np.tile((1.0+0.4*np.sin(lon-omega*t)*np.cos(lat))*F, (nz, 1, 1))

  # Shift state
  xx_xp1 = np.roll(xx, shift=(-1), axis=(2))
  xx_xm1 = np.roll(xx, shift=(1), axis=(2))
  xx_xm2 = np.roll(xx, shift=(2), axis=(2))
  xx_yp1 = np.roll(xx, shift=(-1), axis=(1))
  xx_ym1 = np.roll(xx, shift=(1), axis=(1))

  # Usual L95 in x direction
  xxTen = ((xx_xp1-xx_xm2)*xx_xm1-xx+FF)*cMask

  # Diffusion in x direction
  xxTen += nu*(xx_xp1+xx_xm1-2.0*xx)*cMask

  # Diffusion in y direction
  xxTen += nu*(xx_yp1+xx_ym1-2.0*xx)*yMask

  return xxTen

def stepTL(params, lon, lat, cMask, yMask, t, xxTraj, dx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, cMask, yMask, t, xxTraj)*0.5*dti

  # First step
  dxTmp = dx+tendencyTL(params, cMask, yMask, xxTraj, dx)*0.5*dti

  # Second step
  dx += tendencyTL(params, cMask, yMask, xxTrajTmp, dxTmp)*dti

def tendencyTL(params, cMask, yMask, xxTraj, dx):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  nu = params["nu"]

  # Number of levels
  nz = dx.shape[0]

  # Check horizontal dimensions
  if dx.shape[1] != ny:
    print("inconsistent ny dimension")
    exit(1)
  if dx.shape[2] != nx:
    print("inconsistent nx dimension")
    exit(1)

  # Create tendency
  dxTen = np.zeros((nz,ny,nx))

  # Shift trajectory
  xxTraj_xp1 = np.roll(xxTraj, shift=(-1), axis=(2))
  xxTraj_xm1 = np.roll(xxTraj, shift=(1), axis=(2))
  xxTraj_xm2 = np.roll(xxTraj, shift=(2), axis=(2))
  xxTraj_yp1 = np.roll(xxTraj, shift=(-1), axis=(1))
  xxTraj_ym1 = np.roll(xxTraj, shift=(1), axis=(1))

  # Shift increment
  dx_xp1 = np.roll(dx, shift=(-1), axis=(2))
  dx_xm1 = np.roll(dx, shift=(1), axis=(2))
  dx_xm2 = np.roll(dx, shift=(2), axis=(2))
  dx_yp1 = np.roll(dx, shift=(-1), axis=(1))
  dx_ym1 = np.roll(dx, shift=(1), axis=(1))

  # Usual L95 in x direction
  dxTen = ((dx_xp1-dx_xm2)*xxTraj_xm1+(xxTraj_xp1-xxTraj_xm2)*dx_xm1-dx)*cMask

  # X-direction diffusion
  dxTen += nu*(dx_xp1+dx_xm1-2.0*dx)*cMask

  # Y-direction diffusion
  dxTen += nu*(dx_yp1+dx_ym1-2.0*dx)*yMask

  return dxTen

def stepAD(params, lon, lat, cMask, yMask, t, xxTraj, dx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, cMask, yMask, t, xxTraj)*0.5*dti

  # Second step
  dxTmp = tendencyAD(params, cMask, yMask, xxTrajTmp, dx*dti)
  dx += dxTmp

  # First step
  dx += tendencyAD(params, cMask, yMask, xxTraj, dxTmp*0.5*dti)

def tendencyAD(params, cMask, yMask, xxTraj, dxTen):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  nu = params["nu"]

  # Number of levels
  nz = dxTen.shape[0]

  # Check horizontal dimensions
  if dxTen.shape[1] != ny:
    print("inconsistent ny dimension")
    exit(1)
  if dxTen.shape[2] != nx:
    print("inconsistent nx dimension")
    exit(1)

  # Create increment
  dx = np.zeros((nz,ny,nx))

  # Shift trajectory
  xxTraj_xp1 = np.roll(xxTraj, shift=(-1), axis=(2))
  xxTraj_xm1 = np.roll(xxTraj, shift=(1), axis=(2))
  xxTraj_xm2 = np.roll(xxTraj, shift=(2), axis=(2))
  xxTraj_yp1 = np.roll(xxTraj, shift=(-1), axis=(1))
  xxTraj_ym1 = np.roll(xxTraj, shift=(1), axis=(1))

  # Create shifted increments
  dx_xp1 = np.zeros((nz,ny,nx))
  dx_xm1 = np.zeros((nz,ny,nx))
  dx_xm2 = np.zeros((nz,ny,nx))
  dx_yp1 = np.zeros((nz,ny,nx))
  dx_ym1 = np.zeros((nz,ny,nx))

  # Usual L95 in x direction
  dx_xp1 += dxTen*xxTraj_xm1*cMask
  dx_xm2 -= dxTen*xxTraj_xm1*cMask
  dx_xm1 += dxTen*(xxTraj_xp1-xxTraj_xm2)*cMask
  dx -= dxTen*cMask

  # X-direction diffusion
  dx_xp1 += nu*dxTen*cMask
  dx_xm1 += nu*dxTen*cMask
  dx -= 2.0*nu*dxTen*cMask

  # Y-direction diffusion
  dx_yp1 += nu*dxTen*yMask
  dx_ym1 += nu*dxTen*yMask
  dx -= 2.0*nu*dxTen*yMask

  # Un-shift and accumulate shifted increments
  dx += np.roll(dx_xp1, shift=(1), axis=(2))
  dx += np.roll(dx_xm1, shift=(-1), axis=(2))
  dx += np.roll(dx_xm2, shift=(-2), axis=(2))
  dx += np.roll(dx_yp1, shift=(1), axis=(1))
  dx += np.roll(dx_ym1, shift=(-1), axis=(1))

  return dx
