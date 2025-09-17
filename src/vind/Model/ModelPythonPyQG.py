import numpy as np
import pyqg
import copy

def constructor(data):
  # Model constant parameters
  L =  1000.e3                  # Length scale of box    [m]
  nz = 3                        # Number of layers
  H1 = 500.                     # Layer 1 thickness  [m]
  H2 = 1750.                    # Layer 2
  H3 = 1750.                    # Layer 3
  U1 = 0.05                     # Layer 1 zonal velocity [m/s]
  U2 = 0.025                    # Layer 2
  U3 = 0.00                     # Layer 3
  rho1 = 1025.                  # Layer 1 density
  rho2 = 1025.275               # Layer 2
  rho3 = 1025.640               # Layer 3
  rek = 1.e-7                   # Linear bottom drag coeff [s^-1]
  f0  = 0.0001236812857687059   # Coriolis param [s^-1]
  beta = 1.2130692965249345e-11 # Planetary vorticity gradient [m^-1 s^-1]

  # Get time step
  dt = data["tstep"]

  # Get number of threads
  ntd = data["threads"]

  # Get grid size
  nx = data["nx"]

  # Prepare model
  model = pyqg.LayeredModel(nx=nx,
                            nz=nz,
                            U=[U1,U2,U3],
                            V=[0.,0.,0.],
                            L=L,
                            f=f0,
                            beta=beta,
                            H=[H1,H2,H3],
                            rho=[rho1,rho2,rho3],
                            rek=rek,
                            dt=dt,
                            tmax=0.0,
                            ntd=ntd,
                            log_level=0)

  print("Info     : ModelPythonPyQG constructor done on python side")
  return model

def step(model, data):
  # Get vorticity field
  model.q = data["air_upward_absolute_vorticity"]

  # Check number of grid-points
  nx = model.q.shape[1];
  if nx != model.nx:
     raise ValueError('wrong number of grid-points')

  # Check number of levels
  if model.q.shape[0] != model.nz:
     raise ValueError('wrong number of levels')

  # Run model
  model.tmax += model.dt
  model.run()

  # Reshape
  data["air_upward_absolute_vorticity"] = model.q
