# Stupid-Model
#### A *BUG*gy simulation!

I threw this together in my second year (I think) of college  in a weekend. Hence the terrible code quality and lack of any source control before now.

I thought that I should slap some source control on it before it gets lost in space and time.

##### The program is fairly simple: You enter the number of bugs and predators; the bugs eat the food in the cells and the predators eat the bugs whenever they can.

### Rules
- Bugs move to any adjacent and unoccupied cell each turn.
- Bugs consume all the food in the cell they move into (up to the set limit)
- Cells produce a certain amount of food each turn, as specified in the `Stupid_Cell.DATA` file.
- Predators move randomly each turn into an adjecent and unnocupied cell, *unless* there is an adjecent cell with a bug in it (in which case the predator moves into the bug's cell and consumes the bug).
- A bug increases its size by half of the food it eats; Upon reaching a size of 10, the bug dies and hatches 5 offspring.
- A bug as a 5% chance of randomly dying each turn (producing no offspring).
- The simulation ends when there are no bugs left or the simulation reaches step 1000.
