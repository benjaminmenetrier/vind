#!/usr/bin/env python3
#
# Copyright (c) 2025 Meteorologisk Institutt
#
# Run and plot the results of the Diffusive Diurnal Lorenz 95 model

import numpy as np
from matplotlib import pyplot as plt, cm
from datetime import datetime, timedelta
import sys
sys.path.insert(1, "../src/vind/Python")
from ModelPythonDDL95 import constructor, step

# Constructor
nx = 80
ny = 40
dt = 3600.0

initData = {}
initData["nx"] = nx
initData["ny"] = ny
initData["time step"] = dt
model = constructor(initData)

# Initial condition
stepData = {}
stepData["state"] = {}
stepData["state"]["v"] = np.random.normal(0.0, 1.0, (1, ny, nx))

# Coordinates
Y, X = np.meshgrid(model["x"], model["y"])

# Integration
currentTime = datetime(2025,9,25,0,0,0)

for it in range(72): 
  stepData["year"] = currentTime.year
  stepData["month"] = currentTime.month
  stepData["day"] = currentTime.day
  stepData["hour"] = currentTime.hour
  stepData["minute"] = currentTime.minute
  stepData["second"] = currentTime.second
  step(model, stepData)
  currentTime += timedelta(seconds=dt)
  print(currentTime)

fig = plt.figure(figsize = (11,7), dpi=100)
ax = fig.add_subplot(111)
surf = ax.contourf(Y,X,stepData["state"]["v"][0,:,:],cmap=cm.viridis)
ax.set_xlabel('$x$')
ax.set_ylabel('$y$')
plt.colorbar(surf, orientation="horizontal")
plt.show()
