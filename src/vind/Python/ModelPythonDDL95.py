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
  model["x"] = np.linspace(0.0,2.0*np.pi,initData["nx"],endpoint=False)
  model["y"] = np.linspace(0.0,2.0*np.pi,initData["ny"],endpoint=False)
  model["F"] = 8.0
  model["omega"] = 2.0*np.pi/(24.0*3600.0)
  model["nu"] = 1.0

  # Time-step
  dt = initData["time step"]

  # Internal time-step
  dti = dt/36000.0
  
  # Internal sub-time-step
  model["dti_sub"] = 0.02

  # Number of internal sub-time-steps
  model["nsub"] = int(dti/model["dti_sub"])

  # Sub-time-step
  model["dt_sub"] = dt/model["nsub"]
  model["dt_sub_half"] = 0.5*model["dt_sub"]

  return model

def tendency(model, t, v):
  # Parameters
  nx = model["nx"]
  ny = model["ny"]
  F = model["F"]
  omega = model["omega"]
  nu = model["nu"]
  x = model["x"]
  y = model["y"]

  # Number of levels
  nz = v.shape[0]

  # Check horizontal dimensions
  if v.shape[1] != ny:
    print("inconsistent ny dimension")
  if v.shape[2] != nx:
    print("inconsistent ny dimension")

  # Create tendency
  vt = np.zeros((nz,ny,nx))

  for jx in range(nx):
    for jy in range(ny):
      if jx > 1 and jx < nx-1 and jy > 0 and jy < ny-1:
        # Inside computation zone

        # Time-variable forcing
        FF = (1.0+0.4*np.sin(x[jx]-omega*t)*(1.0-np.cos(y[jy])))*F

        for jz in range(nz):
          # Usual L95 in x direction
          vt[jz,jy,jx] = (v[jz,jy,jx+1]-v[jz,jy,jx-2])*v[jz,jy,jx-1]-v[jz,jy,jx]+FF

          # Add diffusion to get larger scales
          vt[jz,jy,jx] += nu*((v[jz,jy,jx+1]-2.0*v[jz,jy,jx]+v[jz,jy,jx-1])+(v[jz,jy+1,jx]-2.0*v[jz,jy,jx]+v[jz,jy-1,jx]))
      else:
        # Outside computation zone
        for jz in range(nz):
          vt[jz,jy,jx] = -v[jz,jy,jx]
  return vt

def step(model, stepData):
  # Get number of seconds since 00:00:00
  t = stepData["hour"]*3600.0+stepData["minute"]*60.0+stepData["second"]

  # Get integration parameters
  dti_sub = model["dti_sub"]
  nsub = model["nsub"]
  dt_sub = model["dt_sub"]
  dt_sub_half = model["dt_sub_half"]

  # Integrate over sub-time-steps with a RK2 scheme
  for jt in range(nsub):
    # Update all variables
    for k,v in stepData["state"].items():
      vt = tendency(model, t, v)
      vi = v+0.5*dti_sub*vt
      vtt = tendency(model, t+dt_sub_half, vi)
      v += dti_sub*vtt

    # Update time
    t += dt_sub

def tendencyTL(model, t, v):
  # Parameters
  nx = model["nx"]
  ny = model["ny"]
  F = model["F"]
  nu = model["nu"]
  x = model["x"]
  y = model["y"]

  # Number of levels
  nz = v.shape[0]
  if v.shape[1] != ny:
    print("inconsistent ny dimension")
  if v.shape[2] != nx:
    print("inconsistent ny dimension")

  # Create tendency
  vt = np.zeros((nz,ny,nx))

  # Forcing frequency
  omega = 2.0*np.pi/(24.0*3600.0)

  for jx in range(nx):
    # Periodic X indices
    ixm2 = (jx-2)%nx
    ixm1 = (jx-1)%nx
    ixp1 = (jx+1)%nx

    for jy in range(ny):
      # Periodic Y indices
      iym2 = (jy-2)%ny
      iym1 = (jy-1)%ny
      iyp1 = (jy+1)%ny

      # Time-variable forcing
      FF = (1.0+0.4*np.sin(x[jx]-omega*t)*(1.0-np.cos(y[jy])))*F

      for jz in range(nz):
        # Usual L95 in x direction
        vt[jz,jy,jx] = (v[jz,jy,ixp1]-v[jz,jy,ixm2])*v[jz,jy,ixm1]-v[jz,jy,jx]+FF

        # Add diffusion to get larger scales
        vt[jz,jy,jx] += nu*((v[jz,jy,ixp1]-2*v[jz,jy,jx]+v[jz,jy,ixm1])+(v[jz,iyp1,jx]-2*v[jz,jy,jx]+v[jz,iym1,jx]))

  return vt

def stepTL(model, stepData):
  # Get number of seconds since 00:00:00
  t = stepData["hour"]*3600.0+stepData["minute"]*60.0+stepData["second"]

  # Time-step
  dt = model["dt"]

  # Internal time-step
  dti = dt/36000.0
  
  # Internal sub-time-step
  dti_sub = 0.02

  # Number of internal sub-time-steps
  nsub = int(dti/dti_sub)

  # Sub-time-step
  dt_sub = dt/nsub

  # Integrate over sub-time-steps with a RK2 scheme
  for jt in range(nsub):
    # Update all variables
    for k,v in stepData["fields"].items():
      vt = tendency(model, t, v)
      vi = v+0.5*dti_sub*vt
      vtt = tendency(model, t+0.5*dt_sub, vi)
      v += dti_sub*vtt

    # Update time
    t += dt_sub
