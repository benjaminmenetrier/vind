#!/usr/bin/env python3
# This script returns the eckit, fckit and atlas version of jedi-ci

import argparse
import sys
import yaml

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("package", help="Package name")
parser.add_argument("yaml", help="Yaml location")
args = parser.parse_args()

# Read jedi-ci.yaml
with open(args.yaml, "r") as file:
  try:
    config = yaml.safe_load(file)
  except yaml.YAMLError as exc:
    print(exc)

# Find version of this package
version = "Package not found"
for package in config["specs"]:
  if args.package + "@" in package:
    version = package.replace(args.package + "@", "").split(" ", 1)[0]

# Return version
sys.stdout.write(version)
