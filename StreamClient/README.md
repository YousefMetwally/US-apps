# StreamClient
Library for receiving real-time ultrasound stream from Vivid scanners

This repository contains a pre-release of the StreamClient tools for GE Vivid cardiovascular ultrasound scanners.
Please see LICENSE.txt for licensing information.

These tools will allow an appliction to connect to a Vivid E95 or EchoPAC and receive a live stream of ultrasound data. Data streaming is possible under version 202 of Vivid E95 and EchoPAC under a research option key. Please contact eigil.samset@ge.com if you are interested in testing this functionality.

These tools are made available for testing only. This is not medical device software.

The tools are found as zip files here: https://github.com/GEUltrasound/StreamClient/releases.

The StreamClient zip file contains header and library files that can be integrated in an application running on a windows platform.
The environment variable GPVF_DIR must be set to the directory where the zip file is extracted (containing LibGPVF).

To get started, an example program (StreamClientTest.cpp) is provided. This steps would allow you to compile and test this:
* Open a new c++ project in Microsoft Visual Studio
* Add the StreamClientTest.cpp as an "existing" file to the project
* Set the directory were the StreamClient_prerelease.0.1 zip file was extracted as "Additional Include Directories" in the project
* Add StreamClient.lib or StreamClientd.lib (depending on if you are configuring release or debug build) to "Additional Dependencies" in the project
* Add the directory were the StreamClient_prerelease.0.1 zip file was extracted, pluss the subdirectory bin\debug or bin\release, to the "Additional Library Directories" in the project (e.g. C:\StreamClient_prerelease.0.1\bin\debug)
* Compile the project
* To run the project, the directory containing StreamClient.dll must be in the PATH. Also the environment variable GPVF_DIR must be set (e.g. GPVF_DIR=C:\StreamClient_prerelease.0.1)

The StreamServer zip file contains a test program that allows a test server to be set up. This is convenient to allow testing of the streamclient without a vivid product as a server.

For testing, start StreamServer like this:
StreamServer 6542 3D
or
StreamServer 6542 "example stream files"\3d_dump.raw

Compile the StreamClientTest example and run it like this:
StreamClientTest localhost:6542
