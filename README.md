# GeodesicGraph
<pre>
Construction of geodesic graphs for fast geodesic computation
</pre>
<br/>
# Usage
<pre>
DGG_Precompute.exe [method = 'f' for FastDGG or 's' for SVG] [model(.obj)] [accuracy_control_parameter] [number of threads]  
DGG_Solution.exe [method = 'SSAD' or 'MSAD'] [model(.obj)] [graph_binary_file(.binary)] [source_vertex_id or source_file] [output_distance_file]  
</pre>
<br/>
# Example
<pre>
DGG_Precompute.exe f bunny.obj 0.01 8  
DGG_Solution.exe SSAD bunny.obj bunny_FD0.0100000000_c5.binay 0 bunny.distance.txt  
</pre>
