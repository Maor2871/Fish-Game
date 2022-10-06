#include <raylib.h>
#include <string>
#include <iostream>
#include <random>
#include <cmath>
using namespace std;


// ----- Structues -----


struct fish_path
{
    int speed_x;
    int speed_y;
    int is_moving_right;
    int is_moving_top;
    int current_frames_left;
};


// ----- Grahphics Classes -----


class Location
{
    public:
    
        int x;
        int y;
        
    public:
        
        Location()
        {
            x = 0;
            y = 0;
        }
        
        Location(int new_x, int new_y)
        {
            x = new_x;
            y = new_y;
        }
        
        void set_location(Location location)
        {
            x = location.x;
            y = location.y;
        }
};


class Size
{
    public:
        
        int width;
        int height;
        
    public:
        
        Size()
        {
            width = 0;
            height = 0;
        }
        
        Size(int new_width, int new_height)
        {
            width = new_width;
            height = new_height;
        }
        
        void set_size(Size size)
        {
            width = size.width;
            height = size.height;
        }
};


class Entity
{
    /*
        An entity in the world, with rectanngular frame.
    */
    
    protected:
        
        // The location of the entity on the screen. Reffering to center of the entity.
        Location location;
        
        // The size of the entity on the screen.
        Size size;
        
        // The scale of the entity.
        float scale;
        
        // The rotation of the entity.
        float rotation;
       
    public:
    
        // Constructor.
        Entity(Location new_location, Size new_size, int new_scale, int new_rotation)
        {
            // Set the location, size, scale and rotation of the entity.
            location.set_location(new_location);
            size.set_size(new_size);
            scale = new_scale;
            rotation = new_rotation;
        }
        
        // Getters.
        Location get_location() { return location; }
        Size get_size() { return size; }
        int get_scale() { return scale; }
        int get_rotation() { return rotation; }
};


// Declare on cells before grid entity (Resolves the deadlock of Cell includes GridEntity and GridEntity includes Cell).
class Cell;


class GridEntity : public Entity
{
    /*
        An entity inside a grid.
    */
    
    protected:
    
        // The frame of the entity as a rectangle.
        // Important: In the code, the location, size, scale and rotation of the entity might be changed outside the scope of the class,
        //            therefore rectangular_frame might not be up to date. Please call get_updated_rectangular_frame when using rectangular_frame.
        // Important: The frame rectangle considers 0 rotation!
        Rectangle rectangular_frame;
        
        // The maximum amount of cells the entity can be contained in. Note that if scale has no limit, might be inside all the cells of the grid.
        int max_cells_within;
        
        // The amount of cells currently within.
        int current_amount_of_cells_within;
        
        // The array of cells that the entity is currently within. The counstructor is expecting to receive an emty cells array in the size of max_cells_within.
        Cell* cells_within;
        
    public:
    
        GridEntity(Location new_location, Size new_size, int new_scale, int new_rotation, int new_max_cells_within, Cell* cells_within) : Entity(new_location, new_size, new_scale, new_rotation)
        {
            // Save the maximum amount of cells might be in.
            max_cells_within = new_max_cells_within; 
            
            // Create the cells array.
            cells_within = cells_within;
            
            // Currently within no cells.
            current_amount_of_cells_within = 0;
        }
        
        // The function returns the rectangular frame of the entity as a rectangle (NOT CONSIDERING ROTATION).
        Rectangle get_updated_rectangular_frame()
        {
            // Calculate the frame considering the scale of the entity.
            rectangular_frame = {location.x - ((scale * size.width) / 2), location.y - ((scale * size.height) / 2), scale * size.width, scale * size.height};
            
            // Return the frame as rectangle.
            return rectangular_frame;
        }
        
        // The function returns the array of cells that the entity is currently within.
        Cell* get_cells_within() { return cells_within; }
        
        // The function returns the amount of cells curretnly within.
        int get_amount_of_cells_within() { return current_amount_of_cells_within; }
        
        // The function resets the array indicating the cells in which the entity is currently in.
        void reset_cells_within() { current_amount_of_cells_within = 0;}
        
        // The function receives a cell and adds it to the array of cells that the entity is currently within.
        void add_cell_within(Cell* new_cell_within) { cells_within[current_amount_of_cells_within] = new_cell_within; current_amount_of_cells_within++; }
};


class Cell
{
    /*
        A cell of a grid.
    */
    
    private:
        
        // The maximum amount of enetities the current cell can contain.
        int max_entities;
        
        // The current amount of entities within the cell.
        int entities_counter;
        
        // All the entities within the current cell, as pointers.
        GridEntity* entities;
        
    public:
        
        // Constructor.
        Cell(int new_max_entities)
        {
            // Set the maximum amount of entities the cell can contain.
            max_entities = new_max_entities
            
            // Create the entities pointers array.
            entities = new Entity[max_entities];
            
            // Currently there are 0 entities in the cell.
            entities_counter = 0;
        }
        
        // Add new entity to the cell.
        void add_entity(Entity new_entity)
        {
            entities[entities_counter] = new_entity;
            entities_counter += 1;
        }
        
        // Remove received entity from the cell.
        void remove_entity(GridEntity entity_to_remove)
        {
            // Find the received entity in the entities array.
            for (int i = 0; i < entities_counter; i++)
            {
                // Check if the current entity is the entity to remove.
                if (entity_to_remove == entities[i])
                {
                    // Overide the current entity with the entity at the end of the array.
                    entities[i] = entities[entities_counter - 1];
                    
                    // Free the entity at the end of the array.
                    entities[entities_counter - 1] = NULL;
                    entities_counter -= 1;
                }
            }
        }
        
        // The function returns the current amount of entities in the cell.
        int get_entities_counter() { return entities_counter; }
        
        // The function returns all the entities in the cell.
        GridEntity* get_entities() { return entites; }
};


class Grid
{
    /*
        The grid reduces significantly the amount of collision checks in the world.
        
        Notes:
            - All the entities refering inside the class are expected to be GridEntity.
    */
    
    private:
        
        // The maximum amount of entities a cell can contain.
        int cell_maximum_amount_of_entities;
        
        // The amount of columns in the grid.
        int columns_amount;
        
        // The amount of rows in the grid.
        int rows_amount;
        
        // The width of the grid in pixels.
        int width_pixels;
        
        // The height of the grid in pixels.
        int height_pixels;
        
        // The width of each cell in pixels.
        int cell_width_pixels;
        
        // The height of each cell in pixels.
        int cell_height_pixels;
        
        // An 2d matrix with the cells of the grid.
        Cell* cells;
      
    public:
    
        // Counstructor.
        Grid(int new_columns_amount, int new_rows_amount, int new_cell_maximum_amount_of_entities, int new_width_pixels, int new_height_pixels)
        {
            // Set the amount of rows and columns.
            columns_amount = new_columns_amount;
            rows_amount = new_rows_amount;
            
            // Set the width of the grid in pixels.
            width_pixels = new_width_pixels;
            height_pixels = new_height_pixels;
            
            // Calculate and save the amount of pixels each cell covers.
            cell_width_pixels = width_pixels / columns_amount;
            cell_height_pixels = height_pixels / rows_amount;
            
            // Save the maximum amount of entites a single cell can contain.
            cell_maximum_amount_of_entities = new_cell_maximum_amount_of_entities;
            
            // Declare the cells matrix.
            cells = new Cell[rows_amount][columns_amount];
            
            // Create the cells of the grid.
            for (int row_index = 0; row_index < rows_amount; row_index++)
            {
                for (int column_index = 0; column_index < columns_amount; column_index++)
                {
                    cells[row_index][column_index] = Cell(cell_maximum_amount_of_entities);
                }
            }
        }
        
        // The function receives an entity and adds it to the grid.
        void add_entity(GridEntity new_entity)
        {
            /*
                The rectangles are not rotated. 
                This fact means that a rectangle is within a cell, if and only if the cell is between the y axis boundaries of the rectangle, as well as the x axis boundaries.
            */
            // Get the location and size of the entity.
            Location location = new_entity.get_location();
            Size size = new_entity.get_size();
            int scale = new_entity.get_scale();
            
            // Calculate the current actual size of the entity.
            int width = scale * size.width;
            int height = scale * size.height;
            
            // Calculate the x boundaries.
            int x_boundary_left = location.x - (width / 2);
            int x_boundary_right = x_boundary_left + width;
            
            // Calculate the y boundaries.
            int y_boundary_top = location.y - (height / 2);
            int y_boundary_bottom = y_boundary_top + height;
            
            // Find the left and right columns indexes boundaries.
            int left_column_index_boundary = (int) ceil( (double) x_boundary_left / cell_width_pixels);
            int right_column_index_boundary = (int) ceil( (double) x_boundary_right / cell_width_pixels);
            
            // Find the top and bottom rows indexes boundaries.
            int top_row_index_boundary = (int) ceil( (double) y_boundary_top / cell_height_pixels);
            int bottom_row_index_boundary = (int) ceil( (double) y_boundary_bottom / cell_height_pixels);
            
            // Add the entity to all the cells within those boundaries.
            for (int row_index = top_row_index_boundary; row_index <= bottom_row_index_boundary; row_index++)
            {
                for(int col_index = left_column_index_boundary; col_index <= right_column_index_boundary; col_index++)
                {
                    // Add the entity to the current cell.
                    cells[row_index][col_index].add_entity(new_entity);
                    
                    // Save a reference of the current cell in the entity.
                    new_entity.add_cell_within(cells[row_index][col_index]);
                }
            }
        }
        
        // The function removes an entity from the grid.
        void remove_entity(GridEntity entity_to_remove)
        {
            // How many cells the entity is currently within.
            int amount_of_cells_within = entity_to_remove.get_amount_of_cells_within();
            
            // Get the list of cells that the entity is currently within.
            Cell* cells_within = entity_to_remove.get_cells_within();
            
            // Iterate over the cells within.
            for (int i = 0; i < amount_of_cells_within; i++)
            {
                cells_within[i].remove_entity(entity_to_remove);
            }
            
            // Reset the cells within of the entity to remove.
            entity_to_remove.reset_cells_within();
        }
        
        // The function returns the amount of columns.
        int get_columns_amount() { return columns_amount; }
        
        // The function returns the amount of rows.
        int get_rows_amount() { return rows_amount; }
        
        // Returns the cells matrix.
        Cells* get_cells() { return cells; }
};


class MyGif: public GridEntity
{
    /*
        Load a gif to the screen.
        
        Notes:
            - The gif is loaded as a texture, and manipulated as a texture.
            - The gif doesn't have to be on a grid, an almost identical MyGif class is spared due to unnecessary code overload. Simply the value of max_cells_within can be ignored.
    */
    
    // Define the protected attributes.
    protected:
    
        // The path to the gif.
        const char* file_path;
        
        // An optional attribute, if true on intialization, the gif originaly facing left. Indicates to flip it horizontaly to face right on startup.
        bool is_facing_left_on_startup;
        
        // If true, flips the gif horizontally.
        bool is_flip_horizontal;
        
        // If true, flips the gif vertically.
        bool is_flip_vertical;
        
        // When the gif is loaded, this variable contains the amount of frames this gif has.
        int frames_amount = 0;
        
        // Pointing on the current frame of the gif that is being displayed.
        int current_frame = 0;
        
        // The gif as image.
        Image my_gif_image;
        
        // The gif as texture.
        Texture2D my_gif_texture;
    
    // Define the public methods.
    public:
    
        // Constructor.
        MyGif(const char* new_file_path, Location new_location, Size new_size, int new_scale, int new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell* cells_within) : GridEntity(new_location, new_size, new_scale, new_rotation, max_cells_within, cells_within)
        {

            // Save the path to the gif file.
            file_path = new_file_path;
            
            // True if the gif faces left originaly.
            is_facing_left_on_startup = new_is_facing_left_on_startup;
            
            // Set the texture on initialization to face right.
            if (is_facing_left_on_startup) { is_flip_horizontal = true; }
            else { is_flip_horizontal = false; }
            
            // Do not flip the texture verticaly on initialization.
            is_flip_vertical = false;

            // Create the image instance.
            my_gif_image = LoadImageAnim(file_path, &frames_amount);

            // Create the texture instance.
            my_gif_texture = LoadTextureFromImage(my_gif_image);   
        }
        
        void flip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = false : is_flip_horizontal = true;}
        
        void flip_vertical() {is_flip_vertical = true;}
        
        void unflip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = true : is_flip_horizontal = false;}
        
        void unflip_vertical() {is_flip_vertical = false;}
        
        // The function displays the next frame of the gif.
        void set_next_frame()
        {
            // Containing required data about the next frame.
            int next_frame_data_offset;
            
            // Point on the next frame.
            current_frame++;
            
            // Reset the current frame index if currently displaying the last frame of the gif.
            if (current_frame >= frames_amount) { current_frame = 0; }

            // Get memory offset position for next frame data in image.data.
            next_frame_data_offset = my_gif_image.width * my_gif_image.height * 4 * current_frame;

            // Update GPU texture data with next frame image data.
            UpdateTexture(my_gif_texture, ((unsigned char *) my_gif_image.data) + next_frame_data_offset);
        }
        
        void draw_next_frame()
        {
            // -1 cause flip, 1 do not flips.
            int flip_width = 1, flip_height = 1;
            
            // Check if required flipping.
            if (is_flip_horizontal) {flip_width = -1;}
            if (is_flip_vertical) {flip_height = -1;}
            
            // Crop the gif (we don't want to crop any gif, so just take its original frame).
            Rectangle source = {0, 0, flip_width * my_gif_texture.width, flip_height * my_gif_texture.height};
            
            // Were to draw the gif. The input location is where to put the center on the screen.
            Rectangle destination = {location.x, location.y, scale * size.width, scale * size.width};
            
            // We want the gif to be rotated in relation to its center, and we want that the inputed location in the destination rectangle will be the center.
            Vector2 center = {size.width / 2, size.width / 2};
            
            // Draw the texture.
            DrawTexturePro(my_gif_texture, source, destination, center, rotation, WHITE);
        }
    
        // The function removes the gif from the screen.
        void delte_gif()
        {
            // Remove the texture.
            UnloadTexture(my_gif_texture);
            
            // Remove the image.
            UnloadImage(my_gif_image); 
        }

        Location get_location() {return location;}
};


// ----- Game Classes -----


class Fish : public MyGif
{
    /*
        Represents a fish.
    */
    
    protected:
    
        // The speed of the fish on the x and y axes, pixels/frame;
        float speed_x, speed_y;
        
        // The location boundaries of the fish.
        int left_boundary, right_boundary, top_boundary, bottom_boundary;
       
    public:
        
        // Counstructor.
        Fish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell* cells_within) : MyGif(file_path, new_location, new_size, new_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, cells_within)
        {

            // Set the speed of the fish.
            speed_x = new_speed_x;
            speed_y = new_speed_y;
            
            // Set the boundaries of the fish.
            left_boundary = new_left_boundary;
            right_boundary = new_right_boundary;
            top_boundary = new_top_boundary;
            bottom_boundary = new_bottom_boundary;
        }
        
        // Apply movements (including boundaries check).
        void move_left() 
        {
            if (location.x - speed_x < left_boundary) { boundary_exceed(); }
            else { location.x -= speed_x; flip_horizontal(); }
        }
        
        void move_right() 
        {
            if (location.x + speed_x > right_boundary) { boundary_exceed(); }
            else { location.x += speed_x; unflip_horizontal(); }
        }
        
        void move_up() 
        {
            if (location.y - speed_y < top_boundary) { boundary_exceed(); }
            else { location.y -= speed_y; }
        }
        
        void move_down() 
        {
            if (location.y + speed_y > bottom_boundary) {boundary_exceed(); }
            else { location.y += speed_y; }
        }
        
        // The function receives new boundaries, and sets them as the new boundaries of the fish.
        void update_boundaries(int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary)
        {
            left_boundary = new_left_boundary;
            right_boundary = new_right_boundary;
            top_boundary = new_top_boundary;
            bottom_boundary = new_bottom_boundary;
        }
        
        // The function is being called when an attempt to exceed the boundary occurred.
        void boundary_exceed() {}
};


class MyFish : public Fish
{
    /*
        Represents the fish of the user.
    */
    
    public:
    
        MyFish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell* cells_within) : Fish(file_path, new_location, new_size, new_speed_x, new_speed_y, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, cells_within)
        {
        }
};


class WanderFish : public Fish
{
    /*
        Represents a simple fish wandering in the world.
    */
    
    // Define the private attributes of the wandering fish.
    private:
   
        // The range of speeds the fish can move.
        float min_speed_x, max_speed_x, min_speed_y, max_speed_y;
        
        // The range of steps (frames) for a new generated path.
        int min_path_frames, max_path_frames;
        
        // How many paths left in the paths_stack.
        int path_count_in_paths_stack;
        
        // The index on the next path in paths stack.
        int paths_stack_index;
        
        // The current path of the fish.
        fish_path current_path;
        
        // The paths stack of the current wandering fish.
        fish_path* paths_stack;
   
    public:
        
        // Constructor.
        // new_paths_count_in_paths_stack should state the number of stacks which are saved in the received paths_stack.
        WanderFish(const char* file_path, Location new_location, Size new_size, float new_min_speed_x, float new_max_speed_x, float new_min_speed_y, float new_max_speed_y, int new_min_path_frames, int new_max_path_frames, fish_path* new_paths_stack, int new_paths_count_in_paths_stack, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale,float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell* cells_within) : Fish(file_path, new_location, new_size, 0, 0, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, cells_within)
        {
            // --- Set the available properties values range ---

            // Set the range of speeds on both axes.
            min_speed_x = new_min_speed_x;
            max_speed_x = new_max_speed_x;
            min_speed_y = new_min_speed_y;
            max_speed_y = new_max_speed_y;
            
            // Set the range of a new path frames.
            min_path_frames = new_min_path_frames;
            max_path_frames = new_max_path_frames;
            
            // Set the paths stack, and save the number of elements in it.
            paths_stack = new_paths_stack;
            path_count_in_paths_stack = new_paths_count_in_paths_stack;
            paths_stack_index = 0;
            
            // If there are paths in stack path, pull out the first one to be the current path.
            if (path_count_in_paths_stack > 0) { current_path = paths_stack[0]; }
            
            // Otherwise, generate new path and assign it.
            else { current_path = generate_new_path(); }
            
            // Update the properties of the wander fish to match the properties of the current path.
            match_path_in_fish();
        }
        
        // The function generates the next move of the wandering fish.
        void move()
        {
            // Check if there are more frames left in the current path.
            if (current_path.current_frames_left > 0)
            {
                // Move in the current path direction.
                if (current_path.is_moving_right) { move_right(); }
                else { move_left(); }
                if (current_path.is_moving_top) { move_up(); }
                else { move_down(); }
                
                // Decrease the amount of frames left by 1.
                current_path.current_frames_left -= 1;
            }
            
            // No frames left, need to load new path. Check if there are more paths in the paths stack.
            else if (path_count_in_paths_stack > 0)
            {
                // Load the next path.
                current_path = paths_stack[paths_stack_index];
                match_path_in_fish();
                
                // Update the paths counter.
                path_count_in_paths_stack -= 1;
                
                // Now make a move with the current path.
                move();
            }
            
            // No paths left in the paths stack. Generate new one.
            else { current_path = generate_new_path(); match_path_in_fish(); move(); }
        }
        
        // The function generates new path and returns it.
        fish_path generate_new_path()
        {
            // Create the new path.
            fish_path new_path;
            
            // Generate the amount of frames of the new path;
            new_path.current_frames_left = rand() % (max_path_frames - min_path_frames) + min_path_frames;
            
            // Generate the speed of the fish on each axis.
            new_path.speed_x = rand() % (int) floor(max_speed_x - min_speed_x) + min_speed_x;
            new_path.speed_y = rand() % (int) floor(max_speed_y - min_speed_y) + min_speed_y;
            
            // Define the direction of the new path.
            new_path.is_moving_right = rand() % 2;
            new_path.is_moving_top = rand() % 2;

            return new_path;
        }
        
        // The function match the properties of the current path to the properties of the wander fish.
        void match_path_in_fish()
        {
            // Set the speed of the wander fish.
            speed_x = current_path.speed_x;
            speed_y = current_path.speed_y;
        }
};


// ----- Game Functions -----


// --- Handle collisions ---

// The function handles a collision between to fish.
void handle_collision(Fish fish1, Fish fish2)
{
    cout << "Horray fish collided!";
}


// ----- Main Code -----


int main()
{
	// ----- Initialization -----
	
	// --- Constants ---
	
	// - Screen
	const int SCREEN_WIDTH = 1300;
	const int SCREEN_HEIGHT = 800;
	const char* SCREEN_TITLE = "The Fish";
	const int FPS = 24;
    
    // - Graphics Paths
    const char* PATH_MY_FISH = "Textures/my_fish.gif";
    const char* PATH_WORLD1 = "Textures/Worlds/world1.png";
    const char* PATH_FISH1 = "Textures/Fish/fish1.gif";
    
    // - Game Properties
    const int FISH_POPULATION = 30; 
   
	// --- GUI Initialization ---
	
	// Screen set-up.
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
	
	// Fps declaration.
	SetTargetFPS(FPS);
    
    // Load Textures.
    Texture2D world1 = LoadTexture(PATH_WORLD1);
    
    // Create the grid.
    Grid grid = Grid(5, 3, FISH_POPULATION, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create Entities.
    MyFish my_fish = MyFish(PATH_MY_FISH, Location(world1.width / 2, world1.height / 2), Size(300, 300), 15, 12, 0, 0, 0, 0, 1, 0, false, FISH_POPULATION, & (new Cell[FISH_POPULATION]));
    
    fish_path fish1_path = {10, 1, 1, 1, 1000}; 
    fish_path fish1_paths[] = {fish1_path};
    
    // temp wander fish.
    WanderFish fish1 = WanderFish(PATH_FISH1, Location(200, 400), Size(200, 200), 10 , 30, 1, 10, 250, 1000, fish1_paths, 1, 0, 0, 0, 0, 1, 0, true, FISH_POPULATION, & (new Cell[FISH_POPULATION]));
    
    // Add all the entities to the grid.
    grid.add_entity(my_fish);
    grid.add_entity(fish1);
    
    // Create and setup the camera.
    Camera2D camera = { 0 };
    camera.target = (Vector2){ my_fish.get_location().x, my_fish.get_location().y };
    camera.offset = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    camera.rotation = 0;
    camera.zoom = 0.7;
    
	// ----- Game Loop -----
	
	// As long as the Esc button or exit button were not pressed, continue to the next frame.
	while (!WindowShouldClose())
	{
        // --- Boundaries Management ---
        
        // Set the relevant boundaries for all the fish (its scaling considartions occurs within the fish update boundaries calls).
        // Needs to be updated each frame becuase of the scaling change.
        my_fish.update_boundaries((SCREEN_WIDTH / camera.zoom) / 2, world1.width - (SCREEN_WIDTH / camera.zoom) / 2, (SCREEN_HEIGHT / camera.zoom) / 2, world1.height - (SCREEN_HEIGHT / camera.zoom) / 2);
        fish1.update_boundaries(-100, world1.width + 100, -100, world1.height + 100);
        
        // --- User Input Management ---
        
        // Handle arrow keys strokes. They move the fish in the world.
        if (IsKeyDown(KEY_RIGHT)) {my_fish.move_right();}
        if (IsKeyDown(KEY_LEFT)) {my_fish.move_left();}
        if (IsKeyDown(KEY_UP)) {my_fish.move_up();}
        if (IsKeyDown(KEY_DOWN)) {my_fish.move_down();}
        
        // --- Entities Calculations ---
        
        // temp.
        fish1.move();
        
        // --- Handle Collisions ---
        
        // A reference to the cells array.
        Cell* grid_cells = grid.get_cells();
        
        // The amount of entities currently in the current cell.
        int current_cell_entities_amount;
        
        // The entities array of the current cell.
        GridEntity* entities_in_cell;
        
        // The rectangular frame of the two entities in the loop.
        Rectangle first_entity_rectangle;
        Rectangle second_entity_rectangle;        
        
        // Iterate over the cells of the grid.
        for (int row_index = 0; row_index < grid.get_rows_amount(); row_index++)
        {
            for (int col_index = 0; col_index < grid.get_columns_amount(); col_index++)
            {
                // Save the amount of entities in the current cell.
                current_cell_entities_amount = grid_cells[row_index][column_index].get_entities_counter();
                
                // Get the array of entities in the cell.
                entities_in_cell = grid_cells[row_index][column_index].get_entities();
                
                // Iterate over all the possible entities pairs in the current cell.
                for (int first_entity_index = 0; first_entity_index < current_cell_entities_amount - 1; first_entity_index++)
                {
                    for (int second_entity_index = first_entity_index + 1; second_entity_index < current_cell_entities_amount; second_entity_index++)
                    {
                        // Get the rectangle frame of the two entities.
                        first_entity_rectangle = entities_in_cell[first_entity_index].get_updated_rectangular_frame();
                        second_entity_rectangle = entities_in_cell[second_entity_index].get_updated_rectangular_frame();
                        
                        // Check if the two current entities are overlapping.
                        if (CheckCollisionRecs(first_entity_rectangle, second_entity_rectangle))
                        {
                            // The beauty of overloading.
                            handle_collision(entities_in_cell[first_entity_index], entities_in_cell[second_entity_index])
                        }
                    }
                }
            }
        }
        
        // --- Camera ---
        
        // Camera follows my fish movement.
        camera.target = (Vector2){ my_fish.get_location().x, my_fish.get_location().y };
        
        // --- Prepare Gifs for drawing ---
        
        // Prepare my fish for the next gif frame.
		my_fish.set_next_frame();
        
        // temp.
        fish1.set_next_frame();
        
        // --- Draw ---
        
        BeginDrawing();
            
            // Clear the background.
            ClearBackground(RAYWHITE);
            
            // Everything inside this scope, is being manipulated by the camera.
            // Every drawing outside this scope, will show up on the screen without being transformed by the camera.
            BeginMode2D(camera);
                
                // Draw the background.
                DrawTexture(world1, 0, 0, WHITE);

                // Draw the next gif frame of my fish.
                my_fish.draw_next_frame();
                
                fish1.draw_next_frame();
            
            // The end of the drawings affected by the camera.
            EndMode2D();

        EndDrawing();
	}
	
	// ----- Close Game -----
	
    my_fish.delte_gif();
    
	// Close the game screen.
	CloseWindow();
}