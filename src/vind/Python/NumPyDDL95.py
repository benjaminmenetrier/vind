#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import numpy as np

def step(params, lon, lat, t, xx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # First step
  xxTmp = xx+tendency(params, lon, lat, t, xx)*0.5*dti

  # Second step
  xx += tendency(params, lon, lat, t+0.5*dt, xxTmp)*dti

def tendency(params, lon, lat, t, xx):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  ixMin = params["ixMin"]
  ixMax = params["ixMax"]
  iyMin = params["iyMin"]
  iyMax = params["iyMax"]
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

  for jy in range(ny):
    for jx in range(nx):
      if jx >= ixMin and jx <= ixMax and jy >= iyMin and jy <= iyMax:
        # Inside computation zone

        # Retrieve array indices
        ixp1 = (jx+1)%nx
        ixm2 = (jx-2)%nx
        ixm1 = (jx-1)%nx
        iyp1 = jy+1
        iym1 = jy-1

        # Time-dependent forcing
        FF = (1.0+0.4*np.sin(lon[jy,jx]-omega*t)*np.cos(lat[jy,jx]))*F

        for jz in range(nz):
          # Usual L95 in x direction
          xxTen[jz,jy,jx] = (xx[jz,jy,ixp1]-xx[jz,jy,ixm2])*xx[jz,jy,ixm1]-xx[jz,jy,jx]+FF

          # X-direction diffusion
          xxTen[jz,jy,jx] += nu*(xx[jz,jy,ixp1]-2.0*xx[jz,jy,jx]+xx[jz,jy,ixm1])

          # Y-direction diffusion
          if jy > iyMin and jy < iyMax:
            xxTen[jz,jy,jx] += nu*(xx[jz,iyp1,jx]-2.0*xx[jz,jy,jx]+xx[jz,iym1,jx])

  return xxTen

def stepTL(params, lon, lat, t, xxTraj, dx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, t, xxTraj)*0.5*dti

  # First step
  dxTmp = dx+tendencyTL(params, xxTraj, dx)*0.5*dti

  # Second step
  dx += tendencyTL(params, xxTrajTmp, dxTmp)*dti

def tendencyTL(params, xxTraj, dx):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  ixMin = params["ixMin"]
  ixMax = params["ixMax"]
  iyMin = params["iyMin"]
  iyMax = params["iyMax"]
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

  for jx in range(nx):
    for jy in range(ny):
      if jx >= ixMin and jx <= ixMax and jy >= iyMin and jy <= iyMax:
        # Inside computation zone

        # Retrieve array indices
        ixp1 = (jx+1)%nx
        ixm2 = (jx-2)%nx
        ixm1 = (jx-1)%nx
        iyp1 = jy+1
        iym1 = jy-1

        for jz in range(nz):
          # Usual L95 in x direction
          dxTen[jz,jy,jx] = (dx[jz,jy,ixp1]-dx[jz,jy,ixm2])*xxTraj[jz,jy,ixm1]+(xxTraj[jz,jy,ixp1]-xxTraj[jz,jy,ixm2])*dx[jz,jy,ixm1]-dx[jz,jy,jx]

          # X-direction diffusion
          dxTen[jz,jy,jx] += nu*(dx[jz,jy,ixp1]-2.0*dx[jz,jy,jx]+dx[jz,jy,ixm1])

          # Y-direction diffusion
          if jy > iyMin and jy < iyMax:
            dxTen[jz,jy,jx] += nu*(dx[jz,iyp1,jx]-2.0*dx[jz,jy,jx]+dx[jz,iym1,jx])

  return dxTen

def stepAD(params, lon, lat, t, xxTraj, dx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Compute intermediate trajectory state
  xxTrajTmp = xxTraj+tendency(params, lon, lat, t, xxTraj)*0.5*dti

  # Second step
  dxTmp = tendencyAD(params, xxTrajTmp, dx*dti)
  dx += dxTmp

  # First step
  dx += tendencyAD(params, xxTraj, dxTmp*0.5*dti)

def tendencyAD(params, xxTraj, dxTen):
  # Parameters
  nx = params["nx"]
  ny = params["ny"]
  ixMin = params["ixMin"]
  ixMax = params["ixMax"]
  iyMin = params["iyMin"]
  iyMax = params["iyMax"]
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

  # Create tendency
  dx = np.zeros((nz,ny,nx))

  for jx in range(nx):
    for jy in range(ny):
      if jx >= ixMin and jx <= ixMax and jy >= iyMin and jy <= iyMax:
        # Inside computation zone

        # Retrieve array indices
        ixp1 = (jx+1)%nx
        ixm2 = (jx-2)%nx
        ixm1 = (jx-1)%nx
        iyp1 = jy+1
        iym1 = jy-1

        for jz in range(nz):
          # Usual L95 in x direction
          dx[jz,jy,ixp1] += dxTen[jz,jy,jx]*xxTraj[jz,jy,ixm1]
          dx[jz,jy,ixm2] -= dxTen[jz,jy,jx]*xxTraj[jz,jy,ixm1]
          dx[jz,jy,ixm1] += dxTen[jz,jy,jx]*(xxTraj[jz,jy,ixp1]-xxTraj[jz,jy,ixm2])
          dx[jz,jy,jx] -= dxTen[jz,jy,jx]

          # X-direction diffusion
          dx[jz,jy,ixp1] += nu*dxTen[jz,jy,jx]
          dx[jz,jy,jx] -= 2.0*nu*dxTen[jz,jy,jx]
          dx[jz,jy,ixm1] += nu*dxTen[jz,jy,jx]

          # Y-direction diffusion
          if jy > iyMin and jy < iyMax:
            dx[jz,iyp1,jx] += nu*dxTen[jz,jy,jx]
            dx[jz,jy,jx] -= 2.0*nu*dxTen[jz,jy,jx]
            dx[jz,iym1,jx] += nu*dxTen[jz,jy,jx]

  return dx
