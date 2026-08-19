# SplinFormation
Unreal 5 Terrain Spline Extraction, Metadata, and Assisted Generation Utility

## Purpose
Unreal 5 Landscape/Terrain Splines are a great level design tool, but if you need to use their points for procedural generation, navigation, or runtime calculations they are not accessible. This is particularly challenging when you have created roads or other navigable surfaces with the landscape tools. Unfortunately the degree of terrain manipulation available with landscape splines is NOT available with SplineComponents, so there is a capability gap between the two.

This toolkit allows the extraction and replication of a landscape spline into an Unreal SplineComponente which can be accessed in C++ and BP in your project or directly in the editor.

## Methods

### CopyTerrainSpline(ALandscapeSplineActor* LSA, USplineComponent* Destination);

This method replicates a LandscapeSplineActor into a USplineComponent exactly, following all curves, points and exit angles.

### GenerateOffsetSpline(USplineComponent* Base, USplineComponent* Target, float LateralOffset = 0.f, float ZOffset = 0.f, float CloneDensity = 100.f, bool reverse = false);

This method replicates a SplineComponent into another but with specific adjustment parameters, such as lateral or z scalar offsets, point density, or directional reversal.

## License

Copyright © 2025 Teleograph, LLC.

Distributed under the Apache License version 2.0.
