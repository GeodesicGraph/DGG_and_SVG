Precomputation : DGG_precompute.exe [obj mesh name] [accuracy needed] f 20 [number of threads to run] --> It will output a binary file that contains DGG graph data for solving SSAD.
Solving SSAD    : DGG_LC.exe [obj mesh name] [DGG graph data from precomputation] [source index txt file, e.g. src.txt] dij flt 

For the sources index txt file, the format is as follows. The first line will contain the number of sources for computing SSAD. 
Then, the next lines will contain the source indexes, each source index is in a new line. For example, 1 source with index 0, the file will contain :
1
0
The outputs will be saved in [obj mesh name]_[src_index].txt 