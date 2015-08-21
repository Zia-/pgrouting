#include <boost/config.hpp>
#include <iostream>
#include "postgres.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/astar_search.hpp>

/*

To be able to distinguish the edges (source,target) from the (target,source)
"cost" column weights are set to 1 for (source,target)
"reverse_cost" column weights are set to 2 for (target,source)

*/
using namespace boost;

//We need this code. Follow the pattern. For custom property_map.
enum edge_label_t { edge_label };
enum edge_garbage_t { edge_garbage };
//We need this code. Follow the pattern.
namespace boost {
  BOOST_INSTALL_PROPERTY(edge, label);
  BOOST_INSTALL_PROPERTY(edge, garbage);
}

//----------------------------------------------------------------------------------------------
template < typename UndirectedGraph >
void undirected_graph_labelGraph()
{
  const int V = 3;
  UndirectedGraph undigraph(V);
  //When using typedef then we are giving the whole definition of the data type a new name
  typedef typename graph_traits < UndirectedGraph >::vertex_descriptor verdis;
  //Now we are using this new name to make our object.
  verdis vd;
  typedef typename UndirectedGraph::edge_property_type Weight;
  //defining weight, label and garbage property map
  typename property_map < UndirectedGraph, edge_weight_t >::type
    weight = get(edge_weight, undigraph);
  typename property_map < UndirectedGraph, edge_label_t>::type
    label = get(edge_label, undigraph);
  typename property_map < UndirectedGraph, edge_garbage_t>::type
    garbage = get(edge_garbage, undigraph);
  //We have to use typename whenever wanna use during another template initiation or for typedef
  typename graph_traits < UndirectedGraph >::out_edge_iterator out, out_end;
  typename graph_traits < UndirectedGraph >::edge_iterator e, ei;
  typename graph_traits < UndirectedGraph >::vertex_iterator vi, vend;
  typename graph_traits < UndirectedGraph >::edge_iterator edgei, edgeend, edgei1, edgeend1, edgei2, edgeend2;

  std::deque< typename graph_traits < UndirectedGraph >::vertex_descriptor > vert;
  std::cout << "LABELGRAPH UNDIRECTED GRAPH DEMO\n";
  std::cout << "Edge = Label\n";

  //Inserting vertices
  for (int i=0; i < 19; ++i) {
    vd = vertex(i, undigraph);
    vert.push_back(vd);
  }

  //Inserting edges
  // the "cost" column
  add_edge(vert[1],  vert[2],  Weight(1), undigraph); // id 1
  // id 2 has -1
  // id 3 has -1
  add_edge(vert[2],  vert[5],  Weight(1), undigraph); // id 4
  add_edge(vert[3],  vert[6],  Weight(1), undigraph); // id 5
  add_edge(vert[7],  vert[8],  Weight(1), undigraph); // id 6
  add_edge(vert[8],  vert[5],  Weight(1), undigraph); // id 7
  add_edge(vert[5],  vert[6],  Weight(1), undigraph); // id 8
  add_edge(vert[6],  vert[9],  Weight(1), undigraph); // id 9
  add_edge(vert[5],  vert[10], Weight(1), undigraph); // id 10
  add_edge(vert[6],  vert[11], Weight(1), undigraph); // id 11
  add_edge(vert[10], vert[11], Weight(1), undigraph); // id 12
  add_edge(vert[11], vert[12], Weight(1), undigraph); // id 13
  add_edge(vert[10], vert[13], Weight(1), undigraph); // id 14
  add_edge(vert[9],  vert[12], Weight(1), undigraph); // id 15
  add_edge(vert[4],  vert[9],  Weight(1), undigraph); // id 16
  add_edge(vert[14], vert[15], Weight(1), undigraph); // id 17
  add_edge(vert[16], vert[17], Weight(1), undigraph); // id 18

  // the "reverse_cost" column
  add_edge(vert[2],  vert[1], Weight(2), undigraph); // id 1
  add_edge(vert[3],  vert[2], Weight(2), undigraph); // id 2
  add_edge(vert[4],  vert[3], Weight(2), undigraph); // id 3
  add_edge(vert[5],  vert[2], Weight(2), undigraph); // id 4
  // id 5 has -1
  add_edge(vert[8],  vert[7], Weight(2), undigraph); // id 6
  add_edge(vert[5],  vert[8], Weight(2), undigraph); // id 7
  add_edge(vert[6],  vert[5], Weight(2), undigraph); // id 8
  add_edge(vert[9],  vert[6], Weight(2), undigraph); // id 9
  add_edge(vert[10], vert[5], Weight(2), undigraph); // id 10
  // id 11 has -1
  // id 12 has -1
  // id 13 has -1
  add_edge(vert[13], vert[10], Weight(2), undigraph); // id 14
  add_edge(vert[12], vert[9],  Weight(2), undigraph); // id 15
  add_edge(vert[9],  vert[4],  Weight(2), undigraph); // id 16
  add_edge(vert[15], vert[14], Weight(2), undigraph); // id 17
  add_edge(vert[17], vert[16], Weight(2), undigraph); // id 18

  std::vector<verdis> p(num_vertices(undigraph));
  std::vector<int> d(num_vertices(undigraph));

  //Assign label tag a default value (which is our subgraph column in plpgsql)
for (boost::tie(edgei, edgeend) = edges(undigraph); edgei != edgeend; ++edgei){
	label(*edgei) = -1;
	};
//Assign garbage tag a default value. We will drop it at the end
for (boost::tie(edgei, edgeend) = edges(undigraph); edgei != edgeend; ++edgei){
  garbage(*edgei) = 0;
	};

//graph_id would be our edge label. We will increment it's value accordingly.
int graph_id = 1;
/*This for loop will go through our array containing edge values one after another and
check if there is any edge with -1 value or not. We dont have to run a while loop here
as we know that for loop will run in a definite direction and will thus cover all the
edges. If this for loop is at a position then we can assure that those already traversed
edges in the array have label not equal to -1. Although there could be many remaining edges
in the array which have already been altered to some other label value (except -1).*/
for(boost::tie(edgei, edgeend) = edges(undigraph); edgei != edgeend; ++edgei){
	//Select edge with -1 label value
	if(label(*edgei) == -1){
		//Assign it a new label value which will be our graph_id value
		label(*edgei) = graph_id;
		/*This while loop will run until all the connected edges of our above edge will reassigned this
		defined graph_id label value.*/
		while(true){
			//We need this incre int to break this while loop
			int incre = 0;
			/*Now here we will again run for loop on all the edges to select that edge which has
			label value equals to graph_id and garbage value equals to 0. For the very first
			loop it will select our above selected edge. Later on it will select those edges
			which were connected to our above first edge, because we will alter the garbage
			value of those edges which has already been entertained.*/
			for(boost::tie(edgei1, edgeend1) = edges(undigraph); edgei1 != edgeend1; ++edgei1){
				/*Select that edge whose label value is current graph_id value and garbage value is 0.
				Thus we will only select those edges which has recently been identified to be connected
				to our above defined first edge. Old edges of the same subgraph will no longer be into picture.*/
				if(label(*edgei1) == graph_id && garbage(*edgei1) == 0){
					/*Below two lines will select the source and target value of our above selected edge
					so that later on adjoining edges can be identified.*/
					int source_first = source(*edgei1, undigraph);
					int target_first = target(*edgei1, undigraph);
					/*Now this for loop will again run on all the edges of the graph to find those edges
					whose either start-point or end-point is equals to the start-point of our above selected edge
					or whose either start-point or end-point is equals to the end-point of our above selected edge.*/
					for(boost::tie(edgei2, edgeend2) = edges(undigraph); edgei2 != edgeend2; ++edgei2){
						//Following if-else-if conditions are figuring out the connected edges of out above selected edge
						if(source(*edgei2, undigraph) == source_first){
							label(*edgei2) = graph_id;
						}
						else if(source(*edgei2, undigraph) == target_first){
							label(*edgei2) = graph_id;
						}
						else if(target(*edgei2, undigraph) == source_first){
							label(*edgei2) = graph_id;
						}
						else if(target(*edgei2, undigraph) == target_first){
							label(*edgei2) = graph_id;
						};
					};
					//Alter the garvage value of the above selected edge so that it willnot get entertained again n again.
					garbage(*edgei1) = 1;
				}
				else{
					//Increment incre value to break the while loop.
					++incre;
				};
			};
			/*In each while loop this if condition will be checked and if there is no
			edge remaining connected to our above selected first edge then incre
			value will become the number of edges (as above if else loop will enter into
			else condition that number of times which we have total edges in the graph)*/
			if(incre == num_edges(undigraph)){
				break;
			};
		};
		++graph_id;
	};
};

//How to remove garbage property_map???

for(boost::tie(edgei, edgeend) = edges(undigraph); edgei != edgeend; ++edgei){
	std::cout<< *edgei << " = " << label(*edgei) << std::endl;
};


}



int main()
{
  //Making a property using edge_label_t custom datatype. This custom datatype will hold int values.
  //Look at Line 1, 2, and 3 to see how we are adding three tags to an edge.
  typedef property < edge_garbage_t, int >Garbage; //Line 1
  typedef property < edge_label_t, int, Garbage >Label; //Line 2
  typedef property < edge_weight_t, double, Label >Weight; //Line 3

  typedef adjacency_list < listS, vecS, undirectedS,
    no_property, Weight > UndirectedGraph;
  typedef adjacency_list < listS, vecS, directedS,
    no_property, Weight > DirectedGraph;

  undirected_graph_labelGraph < UndirectedGraph > ();
  return 0;
}
