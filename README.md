## GeodesicGraph

This is the source code for the graph-based discrete geodesic algorithms in the following paper:

```
Fast Construction of Discrete Geodesic Graphs
YOHANES YUDHI ADIKUSUMA, ZHENG FANG, and YING HE, Nanyang Technological University
ACM Trans. Graph., Vol. 39, No. 2, Article 14, Publication date: March 2020.
```

A more efficient method for constructing Discrete Geodesic Graph (DGG)—a sparse graph for computing approximate discrete geodesics on triangle meshes. This code also includes the implementation of Saddle Vertex Graph (SVG).

![alt text](https://github.com/GeodesicGraph/GeodesicGraph/blob/master/error_map_on_anisotropic_mesh.png)

---
### Compling the code

1. Please check `ProjectDependencies.txt` for the configuration of the project

2. The code has been tested on:

	- Windows 10 with Microsoft Visual Studio 2017 and 2015
	
3. The code implements the following commands:

	- `DGG_Precompute` for computing the geodesic graph
	- `DGG_Solution` for computing geodesic distance (single-source-all-destination or multiple-source-all-destination)

---
### Usage of commands

1. To compute the geodesic graph, use the command
	
	```Batchfile
	DGG_Precompute [method] [model] [accuracy_control_parameter] [number_of_threads]
	```
	
	- method: `'f'` for FastDGG or `'s'` for SVG
	- model: `.obj/.off` format mesh file
	- accuracy_control_parameter: an expected relative mean error *`ε`* (`'0.01'` represents *`ε`*`= 1%`)
	- number_of_threads: using multiple threads to accelerate the process

	Example command line:
	
	```Batchfile
	DGG_Precompute f bunny.obj 0.01 8 
	```
	
	This command generates the precomputed geodesic graph `bunny_FD0.0100000000_c5.binary`.


2. To compute the geodesic distance, use the command
 
	```Batchfile
	DGG_Solution [method] [model] [graph_binary_file] [source] [output_distance_file]  
	```
	
	- method: `'SSAD'` for single-source-all-destination or `'MSAD'` for multiple-source-all-destination
	- model: `.obj/.off` format mesh file
	- graph_binary_file: the precomputed geodesic graph in `.binary` format
	- source: 0-based source `vertex id` for single source or source `file name` for multiple sources
	- output_distance_file: a file saving the distance field
	
	Example command line:
	
	```Batchfile
	DGG_Solution SSAD bunny.obj bunny_FD0.0100000000_c5.binary 0 bunny.distance.txt
	```
	
	This command generates the distance field `bunny.distance.txt`.

---
### Citation
Please cite the following paper if you use FastDGG:

```BibTeX
@article{10.1145/3144567,
author = {Adikusuma, Yohanes Yudhi and Fang, Zheng and He, Ying},
title = {Fast Construction of Discrete Geodesic Graphs},
year = {2020},
issue_date = {March 2020},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
volume = {39},
number = {2},
issn = {0730-0301},
url = {https://doi.org/10.1145/3144567},
doi = {10.1145/3144567},
journal = {ACM Trans. Graph.},
month = mar,
articleno = {Article 14},
numpages = {14},
keywords = {geodesic path, Geodesic distance, anisotropic meshes, accuracy-aware window propagation, discrete geodesic graph, polyhedral surfaces, complexity analysis}
}
```

Please cite the following paper if you use SVG:

```BibTeX
@article{10.1145/2508363.2508379,
author = {Ying, Xiang and Wang, Xiaoning and He, Ying},
title = {Saddle Vertex Graph (SVG): A Novel Solution to the Discrete Geodesic Problem},
year = {2013},
issue_date = {November 2013},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
volume = {32},
number = {6},
issn = {0730-0301},
url = {https://doi.org/10.1145/2508363.2508379},
doi = {10.1145/2508363.2508379},
journal = {ACM Trans. Graph.},
month = nov,
articleno = {Article 170},
numpages = {12},
keywords = {shortest path, discrete geodesic, saddle vertex graph}
}
```