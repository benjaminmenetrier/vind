#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import numpy as np

def constructor(initData):
  # Setup model
  model = {}
  model["nx"] = initData["nx"]
  model["ny"] = initData["ny"]
  model["ixMin"] = initData["ixMin"]
  model["ixMax"] = initData["ixMax"]
  model["iyMin"] = initData["iyMin"]
  model["iyMax"] = initData["iyMax"]
  model["lon"] = initData["lon"]
  model["lat"] = initData["lat"]
  model["F"] = 8.0
  model["omega"] = 2.0*np.pi/(24.0*3600.0)
  model["nu"] = 1.0

  # Time-step
  model["dt"] = initData["time step"]

  # Internal time-step
  model["dti"] = model["dt"]/36000.0

  return model

def tendency(model, t, xx):
  # Parameters
  nx = model["nx"]
  ny = model["ny"]
  ixMin = model["ixMin"]
  ixMax = model["ixMax"]
  iyMin = model["iyMin"]
  iyMax = model["iyMax"]
  lon = model["lon"]
  lat = model["lat"]
  F = model["F"]
  omega = model["omega"]
  nu = model["nu"]

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

def step(model, stepData):
  # Get number of seconds since 00:00:00
  t = stepData["hour"]*3600.0+stepData["minute"]*60.0+stepData["second"]

  # Integration parameters
  dt = model["dt"]
  dti = model["dti"]

  # Update all variables
  for var in stepData["state"].keys():
    # Get state
    xx = stepData["state"][var]

    # First step
    xxTmp = xx+tendency(model, t, xx)*0.5*dti

    # Second step
    xx += tendency(model, t+0.5*dt, xxTmp)*dti

def tendencyTL(model, t, xxTraj, dx):
  # Parameters
  nx = model["nx"]
  ny = model["ny"]
  ixMin = model["ixMin"]
  ixMax = model["ixMax"]
  iyMin = model["iyMin"]
  iyMax = model["iyMax"]
  nu = model["nu"]

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

def stepTL(model, stepData):
  # Get number of seconds since 00:00:00
  t = stepData["hour"]*3600.0+stepData["minute"]*60.0+stepData["second"]

  # Integration parameters
  dt = model["dt"]
  dti = model["dti"]

  # Update all variables
  for var in stepData["trajectory"].keys():
    # Get state
    xxTraj1 = stepData["trajectory"][var]

    # Compute intermediate trajectory state
    xxTraj2 = xxTraj1+tendency(model, t, xxTraj1)*0.5*dti

    # Get increment
    dx = stepData["increment"][var]

    # First step
    dxTmp = dx+tendencyTL(model, t, xxTraj1, dx)*0.5*dti

    # Second step
    dx += tendencyTL(model, t+0.5*dt, xxTraj2, dxTmp)*dti

def tendencyAD(model, t, xxTraj, dxTen):
  # Parameters
  nx = model["nx"]
  ny = model["ny"]
  ixMin = model["ixMin"]
  ixMax = model["ixMax"]
  iyMin = model["iyMin"]
  iyMax = model["iyMax"]
  nu = model["nu"]

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

def stepAD(model, stepData):
  # Get number of seconds since 00:00:00
  t = stepData["hour"]*3600.0+stepData["minute"]*60.0+stepData["second"]

  # Integration parameters
  dt = model["dt"]
  dti = model["dti"]

  # Update all variables
  for var in stepData["trajectory"].keys():
    # Get state
    xxTraj1 = stepData["trajectory"][var]

    # Compute intermediate trajectory state
    xxTraj2 = xxTraj1+tendency(model, t, xxTraj1)*0.5*dti

    # Get increment
    dx = stepData["increment"][var]

    # Second step
    dxTmp = tendencyAD(model, t+0.5*dt, xxTraj2, dx*dti)
    dx += dxTmp

    # First step
    dx += tendencyAD(model, t, xxTraj1, dxTmp*0.5*dti)
