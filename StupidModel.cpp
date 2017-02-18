#include "stdafx.h"

#include "SFML/OpenGL.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include "Predator.h"
#include "Histogram.h"

using namespace sf;
 
//overload insertion operator so they we can cleanly output the HabitatCell to the console
ostream& operator<<(ostream& os, const HabitatCell &c) {
	os << "Cell " << (c.hasBug ? "has" : "doesn't have") << " a bug.\n";
	os << "Cell " << (c.hasPredator ? "has" : "doesn't have") << " a predator.\n";
	os << "Cell has " << c.foodAvailability << " food available. \n";
	return os;
}

vector<int> getBugDistribution(vector<Bug> bugs, unsigned int groups = 10 ) {
	vector<int> distro;
	distro.reserve( groups );

	//fill the vector with ints of 0
	for ( auto i = 0; i < groups; i++ ) {
		distro.push_back( 0 );
	}

	//fill the groups with the bugs' sizes
	for ( auto i = 0; i < bugs.size(); i++ ) {
		//get the size of each bug
		int size = floor( bugs[i].getSize() );

		if ( size <= groups ) {
			//increment the size bracket for this size
			distro[size]++;
		}
	}

	return distro;
}

//splits a string by delimiter
vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
	vector<string> elems;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

//create a 2D grid of cells from a file
void createGridFromFile( string path, vector<vector<HabitatCell>> &grid, Vector2u &size) {

	
	ifstream f( path );	//open the file
	string line;
		
	//for each line in the file
	for (auto i = 0; getline(f, line); ++i) {
		
		//ignore header info
		if(line.at(0) == '#')
			continue;
		
		//grid.push_back( vector<HabitatCell>() );
		auto s = split(line, '\t');	//split the line

		if(s[0] == "uGridSize") {
			size = Vector2u(stoi(s[1]), stoi(s[2]));

			//make an empty grid
			for( auto x = 0; x < size.x; x++ ) {

				grid.push_back( std::vector<HabitatCell>() );
				grid[x].reserve( size.x );

				for( auto y = 0; y < size.y; y++ ) {

					grid.at( x ).push_back( HabitatCell(x, y, NULL) );

				}
			}
			continue;
		}

		auto x = stoi( s[0] );
		auto y = stoi( s[1] );
		auto food = stof(s[2]);

		//don't allow entries outside of the grid!
		if(x == size.x || y == size.y)
			continue;

		grid[x][y] = HabitatCell(x, y, food);
	}
}

int main()
{
	srand( time(nullptr) );
	Vector2u GridSize;

	auto noOfBugs = 100u;
	auto noOfPreds = 200u;
	auto maxBugConsumeRate = 1.0f;
	auto initialBugSizeMean = 0.1f, initialBugSizeSD = 0.3f;
	auto updateInterval = 16u;

	unsigned int minSize, meanSize, maxSize;

	try {
		cout << "\nInput desired number of bugs: ";
		cin >> noOfBugs;

		cout << "\nInput bug consume rate: ";
		cin >> maxBugConsumeRate;

		cout << "\nInput desired number of predators: ";
		cin >> noOfPreds;

		cout << "\nInput starting bug size average: ";
		cin >> initialBugSizeMean;

		cout << "\nInput starting bug size standard deviation: ";
		cin >> initialBugSizeSD;

		cout << "\nInput interval between updates (in millseconds) (default = 16): ";
		cin >> updateInterval;
	}
	catch (...) {
		cout << "\nInvalid arguement!\nI'll just use the defaults..."
			 << "\nNumber of bugs: " << noOfBugs
			 << "\nBug consume rate: " << maxBugConsumeRate
			 << "\nNumber of predators: " << noOfPreds
			 << "\nStarting size average: " << initialBugSizeMean
			 << "\nStarting size deviation: " << initialBugSizeSD
			 << "\nUpdate interval: " << updateInterval << endl;
	}
		 
	cout << "\nInitialising...\t";
	vector<vector<HabitatCell> > grid;
	vector<Bug> bugs;
	vector<Predator> predators;

	grid.reserve( GridSize.x );
	bugs.reserve( noOfBugs );
	predators.reserve( noOfPreds );

	createGridFromFile("Stupid_Cell.DATA", grid, GridSize);

	//place bugs randomly in the grid
	for (auto i = 0; i < noOfBugs; i++) {
		int x = rand() % GridSize.x,
			y = rand() % GridSize.y;

		if ( grid[x][y].hasBug != true ) {
			grid[x][y].hasBug = true;
			bugs.push_back( Bug(Vector2i(x,y), 1, &grid, maxBugConsumeRate) );

			//get a random float from 0.0 to twice the SD
			auto deviation = static_cast<float>( rand() / (static_cast<float>(RAND_MAX / (initialBugSizeSD*2))) );

			deviation -= initialBugSizeSD;	//allow deviation to be from -SD to +SD

			bugs.rbegin()->setSize( max(initialBugSizeMean + deviation, 0.0f) );	//don't let size be less than 0
		}
		else {
			--i;
		}
	}

	//place predators randomly in the grid
	for (auto i = 0; i < noOfPreds; i++) {
		int x = rand() % GridSize.x,
			y = rand() % GridSize.y;

		if ( grid[x][y].hasPredator != true ) {
			grid[x][y].hasPredator = true;
			predators.push_back( Predator(Vector2i(x,y), GridSpacing, &grid) );
		}
		else {
			--i;
		}
	}

	Font font;
	font.loadFromFile("kenvector_future.ttf");

	// Create the main window
	RenderWindow window(VideoMode(800, 600, 32), "Stupid model");
	Histogram graph(10, font);

	auto clock = Clock();

	cout << "Done!" << endl;
	ofstream logStream;
	logStream.open( "bugSizes.log" );
	logStream << "\n---------------------- Application Started ----------------" << endl;

	while (window.isOpen())
	{
		// Process events
		Event Event;
		while (window.pollEvent(Event))
		{
			// Close window : exit
			if (Event.type == Event::Closed || (Event.type == Event::KeyPressed && Event.key.code == Keyboard::Escape)) {
				window.close();
				graph.close();
			}
			else if (Event.type == Event::MouseButtonPressed) {
				try {
					auto mousePos = Mouse::getPosition(window);
					cout << grid.at(mousePos.x / GridSpacing).at(mousePos.y / GridSpacing);
				}
				catch (...) {
					cout << "No cell at this position\n";
				}
			}
		}

		if ( clock.getElapsedTime().asMilliseconds() >= updateInterval ) {

			//tell all the HabitatCells to produce food
			for(auto itr = grid.begin(); itr != grid.end(); ++itr)
			{
				for(auto itr2 = itr->begin(); itr2 != itr->end(); ++itr2)
				{
					itr2->ProduceFood();
				}
			}

			//reset min and max so that we can find the new ones below...
			minSize = numeric_limits<unsigned int>().min();
			maxSize = 0u;
			meanSize = 0u;

			//sort the bugs in descending order by size;
			sort(bugs.rbegin(), bugs.rend());

			for(auto i = 0; i < bugs.size(); i++) {
			
				bugs[i].Move(GridSize);
				bugs[i].Grow();

				//if the bug fails the survival probability, it dies
				if ( bugs[i].Mortality() ) {
					auto loc = bugs[i].getPosition();
					grid[loc.x][loc.y].hasBug = false;
					auto nth = bugs.begin() + i--;	//decrement i so that we don't skip next bug
					bugs.erase( nth );
					continue;
				}

				//if the bugs reaches a certain size, kill it and hatch it's eggs
				if ( bugs[i].IsReproducing(&bugs, GridSize)) {
					auto nth = bugs.begin() + i--;	//decrement i so that we don't skip next bug
					bugs.erase( nth );
					continue;
				}


				//...find min, mean and max bug sizes
				auto size = bugs[i].getSize();
				if ( size < minSize )
					minSize = size;
				else if ( size > maxSize )
					maxSize = size;

				//add this bug's size to the average...
				meanSize += size;
			}

			for(auto itr = predators.begin(); itr != predators.end(); ++itr) {
				itr->Hunt(bugs, GridSize);
			}

			//...divide by the number of bugs to get the average size
			meanSize /= noOfBugs;

			logStream << "\nMinimum Bug Size: " << minSize;
			logStream << "\tAverage Bug Size: " << meanSize;
			logStream << "\tMaximum Bug Size: " << maxSize;

			//if no bugs left or 1000 iteration passed...
			static int step;
			if ( bugs.size() == 0 || ++step >= 1000) {
				cout << bugs.size() << " bugs left. " << step << " iterations processed" << endl;
				window.close();
				graph.close();
			}

			window.setTitle("Timestep: " + to_string(step));

			//restart the clock
			clock.restart();
		}

		//prepare frame
		window.clear();

		for (auto x = grid.begin(); x != grid.end(); ++x) {
			for (auto y = x->begin(); y != x->end(); ++y) {
				y->Draw(window);
			}
		}
		
		for(auto itr = bugs.begin(); itr != bugs.end(); ++itr)
		{
			itr->Draw(window);
		}
		for(auto itr = predators.begin(); itr != predators.end(); ++itr)
		{
			itr->Draw(window);
		}

		window.display();

		//update and draw the graph
		graph.Update( getBugDistribution(bugs) );
		graph.Draw( Vector2i(window.getPosition().x + window.getSize().x, window.getPosition().y) );

	} //loop back for next frame

	logStream << "-------------------- Application closing ---------------------" << endl;
	logStream.close();

	cout << "\nSimulation finished.";
	system("timeout 10");
	return EXIT_SUCCESS;
}







