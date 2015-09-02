#include <fstream>
#include <boost/graph/adjacency_list.hpp>

//The obj of this struct will hold the import file data
typedef struct {
  int64_t id;
  int64_t source;
  int64_t target;
} pgr_edge_t;

//If dont wanna use "using namespace boost;" then take "https://github.com/boostorg/graph/blob/master/example/bfs.cpp" help

//We need this code. Follow the pattern. For custom property_map.
enum edge_garbage_t { edge_garbage };
//We need this code. Follow the pattern.
namespace boost {
  BOOST_INSTALL_PROPERTY(edge, garbage);
}

//----------------------------------------------------------------------------------------------
void import_from_file(const std::string &input_file_name, pgr_edge_t *edges, unsigned int *count){
	//Here assigning input_file_name argument to a char variable named file_name
	const char* file_name = input_file_name.c_str();

	//The following chunk of code will handle the errors.
	std::ifstream ifs(file_name);
	if (!ifs) {
		std::cerr << "The file " << file_name << " cannot be opened!" << std::endl;
		exit(1);
	}
	//

	//Here the *count value is 0
	//ifs is holding the first value of each row. Since our sampledata.data's first rows value is 18, so *count is 18.
	ifs >> (*count);
	//Here the *count value is 18, which is the number of edges in the import file "sampledata.data"

	long edge_id;
	unsigned int i = 0;
	while (i < (*count) && ifs >> edge_id){
		//This -1 is the last row of our sampledata.data file, thus inform the f() if the file is done traversing
		if (edge_id == -1) break;
		/*Since edges which we have passed here is an array ie edges[100], so we will
		update its each row in each iteration.*/
		//To feed the edge_id. Since we have already done "ifs >>" above so just using "=" operator
		edges[i].id = edge_id;
		//Assigning source and target for each row or say for each edge
		//ifs value is shifting accordingly after each call to feed source, target etc. See the pattern
		ifs >> edges[i].source;
		ifs >> edges[i].target;
		i++;
	}
	ifs.close();
}

//----------------------------------------------------------------------------------------------

template <class G, class E>
static void graph_add_edge(G &graph, const pgr_edge_t &edge){
	//Adding edge to out graph using passed in edge's source and target values
	boost::add_edge(edge.source, edge.target, graph);
}

//----------------------------------------------------------------------------------------------
template < typename UndirectedGraph >
void undirected_graph_labelGraph_importfile(pgr_edge_t *data_edges, int count){
	  const int V = 3;
	  UndirectedGraph undigraph(V);
	  //When using typedef then we are giving the whole definition of the data type a new name
	  typedef typename boost::graph_traits < UndirectedGraph >::edge_descriptor eddis;
	  //Now we are using this new name to make our object.
	  eddis ed;
	  //defining name and garbage property map (we need these maps to access the properties which we have defined in int main())
	  typename boost::property_map < UndirectedGraph, boost::edge_name_t >::type
	    name = get(boost::edge_name, undigraph);
	  //We are not using "boost::" here as we have already declared this custom boost property above (line 13-18)
	  typename boost::property_map < UndirectedGraph, edge_garbage_t>::type
	    garbage = get(edge_garbage, undigraph);
	  //We have to use typename whenever wanna use template parameters (UndirectedGraph in this case) during another template initiation or for typedef
	  typename boost::graph_traits < UndirectedGraph >::edge_iterator edgei, edgeend, edgei1, edgeend1, edgei2, edgeend2;

	  std::cout << "LABELGRAPH UNDIRECTED GRAPH DEMO FROM IMPORT FILE\n";
	  std::cout << "Edge = Label\n";

	  for (int i=0; i < count; ++i){
		  /*For each row we are passing the corresponding edge info which was there in our
		  import file to our graph_add_edge f() to add it to our undigraph graph */
		  graph_add_edge < UndirectedGraph, eddis > (undigraph, data_edges[i]);
	  }

	  //Assign name tag a default value (which is our subgraph column in plpgsql)
	  for (boost::tie(edgei, edgeend) = edges(undigraph); edgei != edgeend; ++edgei){
	  	name(*edgei) = -1;
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
	  	if(name(*edgei) == -1){
	  		//Assign it a new label value which will be our graph_id value
	  		name(*edgei) = graph_id;
	  		/*This while loop will run until all the connected edges of our above edge will reassigned this
	  		defined graph_id label value.*/
	  		while(true){
	  			//We need this incre int to break this while loop
	  			unsigned int incre = 0;
	  			/*Now here we will again run for loop on all the edges to select that edge which has
	  			label value equals to graph_id and garbage value equals to 0. For the very first
	  			loop it will select our above selected edge. Later on it will select those edges
	  			which were connected to our above first edge, because we will alter the garbage
	  			value of those edges which has already been entertained.*/
	  			for(boost::tie(edgei1, edgeend1) = edges(undigraph); edgei1 != edgeend1; ++edgei1){
	  				/*Select that edge whose label value is current graph_id value and garbage value is 0.
	  				Thus we will only select those edges which has recently been identified to be connected
	  				to our above defined first edge. Old edges of the same subgraph will no longer be into picture.*/
	  				if(name(*edgei1) == graph_id && garbage(*edgei1) == 0){
	  					/*Below two lines will select the source and target value of our above selected edge
	  					so that later on adjoining edges can be identified.*/
	  					unsigned int source_first = source(*edgei1, undigraph);
	  					unsigned int target_first = target(*edgei1, undigraph);
	  					/*Now this for loop will again run on all the edges of the graph to find those edges
	  					whose either start-point or end-point is equals to the start-point of our above selected edge
	  					or whose either start-point or end-point is equals to the end-point of our above selected edge.*/
	  					for(boost::tie(edgei2, edgeend2) = edges(undigraph); edgei2 != edgeend2; ++edgei2){
	  						//Following if-else-if conditions are figuring out the connected edges of out above selected edge
	  						if(source(*edgei2, undigraph) == source_first){
	  							name(*edgei2) = graph_id;
	  						}
	  						else if(source(*edgei2, undigraph) == target_first){
	  							name(*edgei2) = graph_id;
	  						}
	  						else if(target(*edgei2, undigraph) == source_first){
	  							name(*edgei2) = graph_id;
	  						}
	  						else if(target(*edgei2, undigraph) == target_first){
	  							name(*edgei2) = graph_id;
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

	  for(boost::tie(edgei, edgeend) = boost::edges(undigraph); edgei != edgeend; ++edgei){
	  	std::cout<< *edgei << " = " << name(*edgei) << std::endl;
	  };

}

int main()
{
  //Making an object of our struct. This will hold the import file data.
  pgr_edge_t edges[100];
  //Creating a count and initializing, later on we will update its value
  unsigned int count = 0;

  std::string fileName("./sampledata.data");
  /*Here we are passing fileName, edges and count; and later on edges and count will get updated
  which we will pass as an argument into our undirected_graph_labelGraph_importfile function*/
  import_from_file(fileName, edges, &count);

  //Making a property using edge_name_t. This will hold int values.
  //Look at Line 1 and 2 to see how we are adding two tags to an edge.
  typedef boost::property < edge_garbage_t, int >Garbage; //Line 1
  typedef boost::property < boost::edge_name_t, double, Garbage >Name; //Line 2

  typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS,
		  boost::no_property, Name > UndirectedGraph;

  undirected_graph_labelGraph_importfile < UndirectedGraph > (edges, count);
  return 0;
}
