#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Diffusive Diurnal Lorenz 95 model

import torch

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
  xxTen = torch.zeros_like(xx)

  # Time-dependent forcing
  FF = ((1.0+0.4*torch.sin(lon-omega*t)*torch.cos(lat))*F).unsqueeze(0).expand(nz, ny, nx)

  # Shift state
  xx_xp1 = torch.roll(xx, shifts=(-1), dims=(2))
  xx_xm1 = torch.roll(xx, shifts=(1), dims=(2))
  xx_xm2 = torch.roll(xx, shifts=(2), dims=(2))
  xx_yp1 = torch.roll(xx, shifts=(-1), dims=(1))
  xx_ym1 = torch.roll(xx, shifts=(1), dims=(1))

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

  # Step integration without extra parameters
  def stepNoParams(xx):
    # First step
    xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t, xx)*0.5*dti

    # Second step
    xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t+0.5*dt, xxTmp)*dti

    return xxTmp

  # Compute JVP
  _, dxOut = torch.func.jvp(stepNoParams, (xxTraj,), (dx,))

  # Copy output content into dx
  dx[:,:,:] = dxOut[:,:,:]

def stepAD(params, lon, lat, cMask, yMask, t, xxTraj, dx):
  # Integration parameters
  dt = params["dt"]
  dti = params["dti"]

  # Step integration without extra parameters
  def stepNoParams(xx):
    # First step
    xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t, xx)*0.5*dti

    # Second step
    xxTmp = xx+tendency(params, lon, lat, cMask, yMask, t+0.5*dt, xxTmp)*dti

    return xxTmp

  # Request gradient computation
  xxTraj.requires_grad_()

  # Compute adjoint
  dxOut = stepNoParams(xxTraj)
  dxOut.backward(gradient=dx)

  # Copy output content into dx
  dx[:,:,:] = xxTraj.grad[:,:,:]
