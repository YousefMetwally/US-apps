# GE cardiovascular ultrasound loader
[Image3dAPI](https://github.com/MedicalUltrasound/Image3dAPI)-based loader libraries for 3D/4D RAWDICOM files from GE Healthcare Vivid cardiovascular ultrasound scanners.

## Getting started instuctions
* Download latest loader and test data from https://github.com/GEUltrasound/GE_CVUS_Loader/releases
* Unzip & follow instructions in README file for how to register the loader
* Clone and build [Image3dAPI](https://github.com/MedicalUltrasound/Image3dAPI) from Visual Studio
* Use `TestViewer` application to try out the loader. 

## Implementation note
Please be aware that the bounding box rotation matrix is not an identity matrix, but in fact y-axis and z-axis are swapped, which reflects the orientation of the voxel data, which reflects the image orientation described in https://github.com/MedicalUltrasound/Image3dAPI/wiki

## Verification of geometry of images
* Reference images of calibrated phantoms are available in the data directory
* The following phantoms have been scanned: CIRS Model 055 and CIRS Model 055A
* The datasheets with dimensions for these phantoms can be found here: https://www.cirsinc.com/products/ultrasound/zerdine-hydrogel/ultrasound-phantoms-for-2d-3d-evaluation/

Test data can be downloaded from https://github.com/GEUltrasound/TestData

The `EchoPACR4 Open4D Verification Results - GELoader Accuracy.pdf` document contain a measurement accuracy report for using the GE loader in EchoPAC. We plan to perform the same accuracy measurements when integrating 3rd party loaders into EchoPAC.

Copyright (c) 2020, GE Healthcare, Ultrasound.
