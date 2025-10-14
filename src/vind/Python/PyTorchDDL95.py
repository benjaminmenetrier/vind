#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import torch

def step(params, lon, lat, t, xx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # First step
  xxTmp = xx+tendency(params, lon, lat, t, xx)*0.5*dti

  # Second step
  xx += tendency(params, lon, lat, t+0.5*dt, xxTmp)*dti

def stepNoParams(xx):
  # Get parameters from globals()
  params = globals()["params"]
  lon = globals()["lon"]
  lat = globals()["lat"]
  t = globals()["t"]

  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # First step
  xxTmp = xx+tendency(params, lon, lat, t, xx)*0.5*dti

  # Second step
  xxTmp = xx+tendency(params, lon, lat, t+0.5*dt, xxTmp)*dti

  return xxTmp

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
  xxTen = torch.zeros_like(xx)

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
        FF = (1.0+0.4*torch.sin(lon[jy,jx]-omega*t)*torch.cos(lat[jy,jx]))*F

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
  # Copy parameters into globals  
  globals()["params"] = params
  globals()["lon"] = lon
  globals()["lat"] = lat
  globals()["t"] = t

  # Compute JVP
  _, dxOut = torch.func.jvp(stepNoParams, (xxTraj,), (dx,))

  # Copy output content into dx
  dx[:,:,:] = dxOut[:,:,:]

def stepAD(params, lon, lat, t, xxTraj, dx):
  # Copy parameters into globals  
  globals()["params"] = params
  globals()["lon"] = lon
  globals()["lat"] = lat
  globals()["t"] = t

  # Request gradient computation
  xxTraj.requires_grad_()

  # Compute adjoint
  dxOut = stepNoParams(xxTraj)
  dxOut.backward(gradient=dx)

  # Copy output content into dx
  dx[:,:,:] = xxTraj.grad[:,:,:]
