#include <raylib.h>
#include <string>
#include <iostream>
#include <random>
#include <cmath>
using namespace std;


// ----- Basice Graphics classes -----


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
        
        // The maximum scale of the entity.
        float max_scale;
        
        // The rotation of the entity.
        float rotation;
       
    public:
    
        // Constructor.
        Entity(Location new_location, Size new_size, float new_scale, float new_max_scale, int new_rotation)
        {
            // Set the location, size, scale and rotation of the entity.
            location.set_location(new_location);
            size.set_size(new_size);
            scale = new_scale;
            max_scale = new_max_scale;
            rotation = new_rotation;
        }
        
        // Default constructor.
        Entity()
        {
            location = Location(0, 0);
            size = Size(0, 0);
            scale = 0;
            rotation = 0;
        }
        
        // Getters.
        Location get_location() { return location; }
        Size get_size() { return size; }
        float get_scale() { return scale; }
        int get_rotation() { return rotation; }
        
        // Setters.
        // Setters.
        void set_scale(float new_scale)
        {
            if (new_scale > max_scale) { scale = max_scale; }
            else { scale = new_scale; }
        }
};


// ----- Structues -----


// A description of a path.
struct fish_path
{
    int speed_x;
    int speed_y;
    int is_moving_right;
    int is_moving_top;
    int current_frames_left;
};

// A fish_path stack.
struct paths_stack
{
    // The initial location of the stack.
    Location initial_location;
    
    // How many paths in the stack.
    int length;
    
    // An array of the paths.
    fish_path* paths;
    
    // If true the path repeats itself.
    bool is_repeat;
    
    // Flags for the initial location.
    bool is_initial_location;
    bool is_left;
};

// Fish recipe.
struct fish_profile
{
    // - Basic properties.
    
    const char* file_path;
    string fish_type;
    bool is_facing_left_on_startup;
    Size size;
    int max_scaling;
    float min_speed_x;
    float max_speed_x;
    float min_speed_y;
    float max_speed_y;
    int min_frames_per_path;
    int max_frames_per_path;
    
    // - Possible paths stacks
    
    // The amount of paths stacks in the paths stacks array.
    int paths_stacks_amount;
    
    // An array of possible paths stacks.
    paths_stack* paths_stacks;    

    // - Flags
    
    // Indicates on the rarity of the fish. Try to avoid too large numbers. Do not use accuracy greater than 4 points after the decimal point.
    float proportion;   
};


// ----- Advanced Grahphics Classes -----


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
        
        // Grid entities usually interact with each other (on collision for instance). Dynamic cast for "instance of" check, is inefficient and not recommended. Thefore, simply save the entity type as a string.
        string entity_type;

    public:

        // The array of cells, as pointers, that the entity is currently within.
        // cell_within is public due to the "referencing each other" conflic of Cell and GridEntity. The cells that the entity is within, directly manipulates cells_witihn from outside this class.
        Cell** cells_within;

    public:

        // Constructor.
        GridEntity(string new_entity_type, Location new_location, Size new_size, float new_scale, float new_max_scale, int new_rotation, int new_max_cells_within, Cell** new_cells_within) : Entity(new_location, new_size, new_scale, new_max_scale, new_rotation)
        {
            // Save the maximum amount of cells might be within.
            max_cells_within = new_max_cells_within; 
            
            // Create the cells array. Its length is exactly max_cells_within.
            cells_within = new_cells_within;
            
            // Not on the grid yet on initialization.
            current_amount_of_cells_within = 0;

            // The type of the entity as a string.
            entity_type = new_entity_type;
        }
        
        // Defalut constructor.
        GridEntity() : Entity()
        {
            max_cells_within = 0;
            cells_within = NULL;
            current_amount_of_cells_within = 0;
            entity_type = "entity";
        }

        // The function returns the rectangular frame of the entity as a rectangle (NOT CONSIDERING ROTATION).
        Rectangle get_updated_rectangular_frame()
        {
            // Calculate the frame considering the scale of the entity.
            rectangular_frame = {location.x - ((sqrt(scale) * size.width) / 2), location.y - ((sqrt(scale) * size.height) / 2), sqrt(scale) * size.width, sqrt(scale) * size.height};

            // Return the frame as rectangle.
            return rectangular_frame;
        }

        // The function returns the array of cells that the entity is currently within.
        Cell** get_cells_within() { return cells_within; }
        
        // The function returns the amount of cells curretnly within.
        int get_amount_of_cells_within() { return current_amount_of_cells_within; }
        
        string get_entity_type() { return entity_type; }
        
        // The function resets the array indicating the cells in which the entity is currently in.
        void reset_cells_within() { current_amount_of_cells_within = 0;}
        
        // The function is being called when a cell was added to cells_within.
        void cell_within_was_added() { current_amount_of_cells_within++; }
        
        // A GridEntity object might want to handle collision in its original class and not as an GridEntity.
        virtual void handle_collision(GridEntity* colided_with_entity) {}
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
        GridEntity** entities;
        
    public:
        
        // Constructor.
        Cell(int new_max_entities, GridEntity** new_entities)
        {
            // Set the maximum amount of entities the cell can contain.
            max_entities = new_max_entities;
            
            // Initialize the entities array with the size of max_entities.
            entities = new_entities;
            
            // Currently there are 0 entities in the cell.
            entities_counter = 0;
        }
        
        // Default constructor.
        Cell()
        {
            max_entities = 0;
            entities = NULL;
            entities_counter=0;
        }
       
        // Add new entity to the cell.
        void add_entity(GridEntity* new_entity)
        {
            // Add the entity to the cell.
            entities[entities_counter] = new_entity;
            entities_counter += 1;

            // Add the cell to the entity.
            new_entity -> cells_within[new_entity -> get_amount_of_cells_within()] = this;
            new_entity -> cell_within_was_added();
        }
        
        // Remove received entity from the cell.
        void remove_entity(GridEntity* entity_to_remove)
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
                    entities_counter -= 1;
                    
                    // That's it.
                    break;
                }
            }
        }
        
        // The function returns the current amount of entities in the cell.
        int get_entities_counter() { return entities_counter; }
        
        // The function returns all the entities in the cell.
        GridEntity** get_entities() { return entities; }
};


class Grid
{
    /*
        The grid reduces significantly the amount of collision checks in the world.
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
        
        // An 2d matrix with the cells of the grid, as pointers.
        Cell*** cells;
      
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
            cells = new Cell**[rows_amount];
            for(int i = 0; i < rows_amount; i++)
                cells[i] = new Cell*[columns_amount];
            
            // Create the cells of the grid.
            for (int row_index = 0; row_index < rows_amount; row_index++)
            {
                for (int column_index = 0; column_index < columns_amount; column_index++)
                {
                    cells[row_index][column_index] = new Cell(cell_maximum_amount_of_entities, new GridEntity*[cell_maximum_amount_of_entities]);
                }
            }
        }
        
        // The function receives an entity and refresh it location on the grid.
        void refresh_entity(GridEntity* entity_to_refresh)
        {
            // Remove it from the grid.
            remove_entity(entity_to_refresh);
            
            // Add it to the grid.
            add_entity(entity_to_refresh);
        }
        
        // The function receives an entity and adds it to the grid.
        void add_entity(GridEntity* new_entity)
        {
            /*
                The rectangles are not rotated. 
                This fact means that a rectangle is within a cell, if and only if the cell is between the y axis boundaries of the rectangle, as well as the x axis boundaries.
            */
            // Get the location and size of the entity.
            Location location = new_entity -> get_location();
            Size size = new_entity -> get_size();
            float scale = new_entity -> get_scale();
            
            // Calculate the current actual size of the entity.
            int width = (int) floor(sqrt(scale) * size.width);
            int height = (int) floor(sqrt(scale) * size.height);
            
            // Calculate the x boundaries.
            int x_boundary_left = location.x - (int) floor(width / 2);
            int x_boundary_right = location.x + (int) ceil(width / 2);
            
            // Calculate the y boundaries.
            int y_boundary_top = location.y - (int) floor(height / 2);
            int y_boundary_bottom = location.y + (int) ceil(height / 2);
            
            // Find the left and right columns indexes boundaries.
            int left_column_index_boundary = (int) floor( (double) x_boundary_left / cell_width_pixels);
            int right_column_index_boundary = (int) floor( (double) x_boundary_right / cell_width_pixels);
            
            // Find the top and bottom rows indexes boundaries.
            int top_row_index_boundary = (int) floor( (double) y_boundary_top / cell_height_pixels);
            int bottom_row_index_boundary = (int) floor( (double) y_boundary_bottom / cell_height_pixels);
            
            // Don't care if outside the grid.
            if (top_row_index_boundary >= rows_amount) { top_row_index_boundary = rows_amount - 1; }
            else if (top_row_index_boundary < 0) { top_row_index_boundary = 0; }
            
            if (bottom_row_index_boundary < 0) { bottom_row_index_boundary = 0; }
            else if (bottom_row_index_boundary >= rows_amount) { bottom_row_index_boundary = rows_amount - 1; }
            
            if (left_column_index_boundary < 0) { left_column_index_boundary = 0; }
            else if (left_column_index_boundary >= columns_amount) { left_column_index_boundary = columns_amount - 1; }
            
            if (right_column_index_boundary >= columns_amount) { right_column_index_boundary = columns_amount - 1; }
            else if (right_column_index_boundary < 0) { right_column_index_boundary = 0; }

            // Add the entity to all the cells within those boundaries.
            for (int row_index = top_row_index_boundary; row_index <= bottom_row_index_boundary; row_index++)
            {
                for(int col_index = left_column_index_boundary; col_index <= right_column_index_boundary; col_index++)
                {
                    // Add the entity to the current cell (also adds the cell to the current entity).
                    cells[row_index][col_index] -> add_entity(new_entity);
                }
            }
        }
        
        // The function removes an entity from the grid.
        void remove_entity(GridEntity* entity_to_remove)
        {
            // How many cells the entity is currently within.
            int amount_of_cells_within = entity_to_remove -> get_amount_of_cells_within();
            
            // Get the list of cells that the entity is currently within.
            Cell** cells_within = entity_to_remove -> get_cells_within();

            // Iterate over the cells within.
            for (int i = 0; i < amount_of_cells_within; i++)
            {
                cells_within[i] -> remove_entity(entity_to_remove);
            }
            
            // Reset the cells within of the entity to remove.
            entity_to_remove -> reset_cells_within();
        }
        
        // Getters.

        // The function returns the amount of columns.
        int get_columns_amount() { return columns_amount; }
        
        // The function returns the amount of rows.
        int get_rows_amount() { return rows_amount; }
        
        int get_width_pixels() { return width_pixels; }
        int get_height_pixels() { return height_pixels; }
        
        // Returns the cells matrix.
        Cell*** get_cells() { return cells; }

};


class MyGif: public GridEntity
{
    /*
        Load a gif to the screen.
        
        Notes:
            - The gif is loaded as a texture, and manipulated as a texture.
            - The gif doesn't have to be on a grid, an almost identical MyGif class is spared due to unnecessary code overload. Simply the value of max_cells_within can be ignored and the array    
              cells_within should be nothing but a null pointer.
    */
    
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
    
    public:
    
        // Constructor.
        MyGif(const char* new_file_path, string new_entity_type, Location new_location, Size new_size, float new_scale, float new_max_scale, int new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : GridEntity(new_entity_type, new_location, new_size, new_scale, new_max_scale, new_rotation, new_max_cells_within, new_cells_within)
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
        
        // Flipping manipulations
        void flip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = false : is_flip_horizontal = true;}
        void flip_vertical() {is_flip_vertical = true;}
        void unflip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = true : is_flip_horizontal = false;}
        void unflip_vertical() {is_flip_vertical = false;}
        
        // The function Prepare the next frame of the gif.
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
        
        // The function draws the last frame that was set with set_next_frame().
        void draw_next_frame()
        {
            // -1 cause flip, 1 do not flips.
            int flip_width = 1, flip_height = 1;
            
            // Check if required flipping.
            if (is_flip_horizontal) {flip_width = -1;}
            if (is_flip_vertical) {flip_height = -1;}
            
            // Crop the gif (we don't want to crop any gif, so just take its original frame).
            Rectangle source = {0, 0, flip_width * my_gif_texture.width, flip_height * my_gif_texture.height};
            
            // Where to draw the gif. The input location is where to put the center on the screen.
            Rectangle destination = {location.x, location.y, (int) floor(sqrt(scale) * size.width), (int) floor(scale * size.height)};
            
            // We want the gif to be rotated in relation to its center, and we want that the inputed location in the destination rectangle will be the center.
            Vector2 center = {size.width / 2, size.width / 2};
            
            // Draw the next frame of the gif properly.
            DrawTexturePro(my_gif_texture, source, destination, center, rotation, WHITE);
        }

        // The function removes the gif from the screen.
        void delete_gif()
        {
            // Remove the texture.
            UnloadTexture(my_gif_texture);
            
            // Remove the image.
            UnloadImage(my_gif_image);
        }
        
        // Returns the center location of the gif.
        Location get_location() {return location;}
};


// ----- Game Classes -----


// Need to declare for the Fish class.
class FishNetwork;


class Fish : public MyGif
{
    /*
        Represents a fish.
    */
    
    protected:
        
        // The type of the fish.
        string fish_type;
        
        // The speed of the fish on the x and y axes, pixels/frame;
        float speed_x, speed_y;
        
        // The location boundaries of the fish.
        int left_boundary, right_boundary, top_boundary, bottom_boundary;
        
        // Indicating if the fish is out of bounds.
        bool is_fish_out_of_bounds;
        
        // Indicating that the fish was eaten.
        bool is_eaten;
       
    public:
        
        // Counstructor.
        Fish(const char* file_path, string new_fish_type, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_max_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : MyGif(file_path, "Fish", new_location, new_size, new_scale, new_max_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
            // Save the type of the fish.
            fish_type = new_fish_type;
            
            // Set the speed of the fish.
            speed_x = new_speed_x;
            speed_y = new_speed_y;
            
            // Set the boundaries of the fish.
            left_boundary = new_left_boundary;
            right_boundary = new_right_boundary;
            top_boundary = new_top_boundary;
            bottom_boundary = new_bottom_boundary;
            
            // Set false in initialization. Update accurate value on the first move call.
            is_fish_out_of_bounds = false;
            
            // The fish is not eaten on startup.
            is_eaten = false;
        }
                
        // Apply movements (including boundaries check).
        void move_left() 
        {
            if (location.x - speed_x < left_boundary) { boundary_exceed(); is_fish_out_of_bounds = true; }
            else { location.x -= speed_x; flip_horizontal(); }
        }
        
        void move_right() 
        {
            if (location.x + speed_x > right_boundary) { boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.x += speed_x; unflip_horizontal(); }
        }
        
        void move_up() 
        {
            if (location.y - speed_y < top_boundary) { boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.y -= speed_y; }
        }
        
        void move_down() 
        {
            if (location.y + speed_y > bottom_boundary) {boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.y += speed_y; }
        }
        
        // The function receives new boundaries, and sets them as the new boundaries of the fish.
        void update_boundaries(int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, bool is_consider_size)
        {
            // Note: the boundaries are compared to the current location of the fish, which consideres its center. Therefore the precise boundary would be calculated with half the current dimentions of the fish.
            if (is_consider_size)
            {
                left_boundary = new_left_boundary - (int) floor(sqrt(scale) * size.width / 2);
                right_boundary = new_right_boundary + (int) ceil(sqrt(scale) * size.width / 2);
                top_boundary = new_top_boundary - (int) floor(sqrt(scale) * size.height / 2);
                bottom_boundary = new_bottom_boundary + (int) ceil(sqrt(scale) * size.height / 2);
            }
            
            else
            {
                left_boundary = new_left_boundary;
                right_boundary = new_right_boundary;
                top_boundary = new_top_boundary;
                bottom_boundary = new_bottom_boundary;
            }
        }
        
        // The function is being called when the fish is out of bounds.
        void boundary_exceed() {  }
        
        // Returns true if the fish is out of bounds.
        bool get_is_fish_out_of_bounds() { return is_fish_out_of_bounds; }
        
        // Getters.
        Size get_size() { return size; }
        string get_fish_type() { return fish_type; }
        bool get_is_eaten() { return is_eaten; }
        
        // The function deletes the fish from the world.
        // If the gif is not a part of a network, it is required to manualy remove it from the grid.
        void delete_fish()
        {
            // delete_gif();
        }
        
        // The function receives the amount of pixels were eaten by the fish (simply calculation of width, height and scale of the eaten entity), and increases the size of the fish.
        void eat (int pixels)
        {
            // How many pixels are required to increase the width and the height by 1 pixel (the 4 is for the corners).
            int current_pixels_for_loop = (int) floor((2 * size.width * sqrt(scale)) + (2 * size.height * sqrt(scale)) + 4);
            
            // How many full loops can be added.
            int full_loops_counter = 0;
            
            // Count the full loops available, and update the left pixels.
            while (pixels > current_pixels_for_loop)
            {
                pixels -= current_pixels_for_loop;
                current_pixels_for_loop += 4;
                full_loops_counter += 1;
            }
            
            // The new scale is the current width + the added pixels / the original width of the fish.
            scale =  pow(((size.width * sqrt(scale)) + full_loops_counter + ( (double) pixels / current_pixels_for_loop) ) / size.width, 2);
            cout << "calculations: current_pixels_for_loop: " << current_pixels_for_loop << ", full_loops_counter: " << full_loops_counter << ", scale: " << scale << ".\n";
        }
        
        // The function is being called when the fish is getting eaten.
        void eaten()
        {
            // Indicate that the fish is getting eaten.
            is_eaten = true;
        }
        
        // The function handles a collision between the fish and another GridEntity (Note that a collision between two entities is called only once).
        void handle_collision(GridEntity* collided_with_entity)
        {
            //cout << "checking...\n";
            // It's a collision of two fish.
            if (collided_with_entity -> get_entity_type() == "Fish")
            {   
                cout << "collision:" << fish_type << ", " << ((Fish*) collided_with_entity) -> get_fish_type() << ".\n";
                // There is no canibalism in fish-hood.
                if (fish_type == ((Fish*) collided_with_entity) -> get_fish_type())
                {
                    //cout << "same fish.\n\n";
                    // Ignore the collision.
                    return;
                }
                //cout << "different!\n";
                // The size in width of the current fish.
                int my_size = (int) floor(size.width * size.height * scale);
            
                // The size in width of the received fish.
                int other_size = (int) floor(collided_with_entity -> get_size().width * collided_with_entity -> get_size().height * collided_with_entity -> get_scale());
                
                cout << "my type: " << fish_type << ", my width: " << size.width * sqrt(scale) << "\n";
                cout << "other type: " << ((Fish*) collided_with_entity) -> get_fish_type() << ", other width: " << collided_with_entity -> get_size().width * sqrt(collided_with_entity -> get_scale()) << "\n";
                
                // Check which fish is larger.
                if (my_size > other_size)
                {
                    // The current fish is getting bigger.
                    eat(other_size);
                    
                    cout << "my fish new width: " << size.width * sqrt(scale) << "\n\n";

                    // The other fish is getting eaten.
                    ((Fish*) collided_with_entity) -> eaten();
                }
                
                else
                {
                    // The other fish is getting bigger.
                    ((Fish*)collided_with_entity) -> eat(my_size);
                    cout << "other fish new width: " << (collided_with_entity -> get_size().width) * sqrt(collided_with_entity -> get_scale()) << "\n\n";
                    // The current fish is getting eaten.
                    eaten();
                }
            }           
        }
};


class MyFish : public Fish
{
    /*
        Represents the fish of the user.
    */
    
    public:
    
        MyFish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_max_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : Fish(file_path, "my fish", new_location, new_size, new_speed_x, new_speed_y, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_max_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
        }
};


class WanderFish : public Fish
{
    /*
        Represents a simple fish wandering in the world.
    */
    
    private:

        // The range of steps (frames) for a new generated path.
        int min_path_frames, max_path_frames;

        // The index on the next path in paths stack.
        int paths_stack_index;
        
        // The current path of the fish.
        fish_path current_path;
        
        // Indicating the orginal frames amount the current path had, necessary for later reset (relevant only with paths_stack, repeat on).
        int current_path_original_frames_amount;
        
        // The paths stack of the current wandering fish.
        paths_stack my_paths_stack;
        
        // The range of speeds in both axes.
        int min_speed_x;
        int max_speed_x;
        int min_speed_y;
        int max_speed_y;
   
    public:
        
        // Constructor.
        // new_paths_count_in_paths_stack should state the number of stacks which are saved in the received paths_stack.
        // The location is relevant if there is no paths_stack or is_initial_location is false;
        WanderFish(const char* file_path, string new_fish_type, Location new_location, bool is_initial_left_location, Size new_size, float new_min_speed_x, float new_max_speed_x, float new_min_speed_y, float new_max_speed_y, int new_min_path_frames, int new_max_path_frames, paths_stack new_paths_stack, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_max_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : Fish(file_path, new_fish_type, new_location, new_size, 0, 0, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_max_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
            // Set the range of speeds on both axes.
            min_speed_x = new_min_speed_x;
            max_speed_x = new_max_speed_x;
            min_speed_y = new_min_speed_y;
            max_speed_y = new_max_speed_y;
            
            // Set the range of a new path frames.
            min_path_frames = new_min_path_frames;
            max_path_frames = new_max_path_frames;
            
            // Set the paths stack, and save the number of elements in it.
            my_paths_stack = new_paths_stack;
            paths_stack_index = 0;
            
            // If there are paths in stack path, pull out the first one to be the current path.
            if (my_paths_stack.length > 0) 
            {
                // If initial location is relevant.
                if (my_paths_stack.is_initial_location) { location.set_location(my_paths_stack.initial_location); }
                
                // Otherwise, randomize it.
                else { set_random_initial_location(is_initial_left_location); }
                
                // Set the first path as the current path.
                current_path = my_paths_stack.paths[0];
                
                // Remember the original amount of frames.
                current_path_original_frames_amount = current_path.current_frames_left;
            }
            
            // Otherwise, generate new path and assign it.
            else 
            {
                current_path = generate_new_path();
                
                set_random_initial_location(is_initial_left_location);
            }
            
            // Update the properties of the wander fish to match the properties of the current path.
            match_path_in_fish();
        }
        
        // The function sets a random initial_location.
        void set_random_initial_location(bool is_initial_left_location)
        {
            // Randomize the location of the y axis.
            int y_coordinates;
            if (bottom_boundary - top_boundary <= 0) { y_coordinates = top_boundary + size.height; }
            else { y_coordinates = rand() % (bottom_boundary - top_boundary) + top_boundary; }

            // The location should be at the left side of the world.
            if (is_initial_left_location)
            {
                location.set_location(Location(left_boundary + size.width, y_coordinates));
            }
            
            // The location should be at the right side of the world.
            else
            {
                location.set_location(Location(right_boundary - size.width, y_coordinates));
            }
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
            else if (paths_stack_index < my_paths_stack.length - 1)
            {
                // First reset the current path for later use.
                my_paths_stack.paths[paths_stack_index].current_frames_left = current_path_original_frames_amount;
                
                // Point on the next path.
                paths_stack_index++;

                // Load the next path.
                current_path = my_paths_stack.paths[paths_stack_index];
                match_path_in_fish();
                
                // Save its original frames amount for later reset.
                current_path_original_frames_amount = current_path.current_frames_left;

                // Make a move with the current path.
                move();
            }
            
            // No paths left in the paths stack.
            else 
            {
                // Check if the paths stack is set to repeat.
                if (my_paths_stack.is_repeat)
                {                   
                    // First reset the current path for later use.
                    my_paths_stack.paths[paths_stack_index].current_frames_left = current_path_original_frames_amount;
                    
                    // Reset the paths stack index.
                    paths_stack_index = 0;
                    
                    // Load the next path.
                    current_path = my_paths_stack.paths[paths_stack_index];
                    match_path_in_fish();

                    // Save its original amount of frames for later reset.
                    current_path_original_frames_amount = current_path.current_frames_left;
                }
                
                // No repeat.
                else
                {                    
                    // Generate new path.
                    current_path = generate_new_path();
                    match_path_in_fish();
                }
                
                // We are set to apply the next move.
                move(); 
            }
        }
        
        // The function generates new path and returns it.
        fish_path generate_new_path()
        {
            // Create the new path.
            fish_path new_path;
            
            // Generate the amount of frames of the new path;
            if (max_path_frames - min_path_frames <= 0) { new_path.current_frames_left = min_path_frames; }
            else { new_path.current_frames_left = rand() % (max_path_frames - min_path_frames) + min_path_frames; }
            
            // Generate the speed of the fish on each axis.
            if (max_speed_x - min_speed_x <= 0) { new_path.speed_x = min_speed_x; }
            else { new_path.speed_x = rand() % (int) floor(max_speed_x - min_speed_x) + min_speed_x; }
            if (max_speed_y - min_speed_y <= 0) { new_path.speed_y = min_speed_y; }
            else { new_path.speed_y = rand() % (int) floor(max_speed_y - min_speed_y) + min_speed_y; }
            
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


class FishNetwork
{
    /*
        Manages the fish in the world.
    */
    
    private:
    
        // The maximum fish population in the world.
        int max_population;
        
        // The current population.
        int current_population;
        
        // FishNetwork updates its grid with the relevant information.
        Grid* grid;
        
        // An array of fish profiles must be loaded on startup.
        fish_profile* fish_on_startup;
        int fish_on_startup_length;
        
        // An array of fish profiles to load when available.
        fish_profile* available_fish;
        int available_fish_length;
        
        // The current fish in the fish network, as pointers.
        WanderFish** fish;
        int current_fish_amount;
        
        // This array is like a lot. On a given random number, each cell in the array states the range of numbers indicating on the matching available fish in the available_fish array.
        // See the implemantation in the constructor for further explanation.
        int* proportions_lot;
        int lot_range;
        
    public:
    
        // Constructor.
        FishNetwork(int new_max_population, Grid* new_grid, fish_profile* new_fish_on_startup, int new_fish_on_startup_length, fish_profile* new_available_fish, int new_available_fish_length)
        {
            // The max population.
            max_population = new_max_population;
            
            // The current population.
            current_population = 0;
            
            // Save the grid on which the fish network is happening.
            grid = new_grid;
            
            // The fish profiles to load on startup.
            fish_on_startup = new_fish_on_startup;
            fish_on_startup_length = new_fish_on_startup_length;
            
            // The available fish profiles to load lively.
            available_fish = new_available_fish;
            available_fish_length = new_available_fish_length;
            
            // Create the fish array.
            fish = new WanderFish*[max_population];
            current_fish_amount = 0;
            
            // Create the proportions lot.
            
            // The lot range states the boundaries of the random generated number. Being calculated in the following loop.
            lot_range = 0;
            
            // Initialize the proportions lot array.
            proportions_lot = new int[available_fish_length];
            
            // Iterate over the available fish proportions.
            for (int i = 0; i < available_fish_length; i++)
            {
                proportions_lot[i] = available_fish[i].proportion * 1000;
                lot_range += proportions_lot[i];
            }
        }
        
        // The function creates and loads all the fish on startup.
        void setup()
        {           
            // Load all the fish on startup.
            for (int i = 0; i < fish_on_startup_length; i++)
                load_fish_profile(fish_on_startup[i]);
            
            // Add fish to the fish network, to fill the max fish population.
            load_available_fish();
        }
        
        // The function loads available fish up to the max fish population.
        void load_available_fish()
        {
            // Keep loading fish up to the max population.
            while (current_fish_amount < max_population)
            {               
                // lot the next available fish.
                int random_lot = rand() % lot_range;
                
                // Spot the lotted fish, and load it.
                for (int i = 0; i < available_fish_length; i++)
                {
                    // That's the fish to load.
                    if (random_lot < proportions_lot[i])
                    {
                        // Load the lotted fish.
                        load_fish_profile(available_fish[i]);
                        break;
                    }
                }
            }
        }
        
        // The function updates the boundaries of all the fish.
        void update_boundaries(int left, int right, int top, int bottom, bool is_consider_size)
        {
            // Iterate over the current fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                // Update the boundaries of the current fish (taking the scaling into considerations)
                fish[i] -> update_boundaries(left, right, top, bottom, is_consider_size);
            }
        }
        
        // The function deletes all the eaten fish.
        void handle_eaten()
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                // If the current fish is eaten.
                if (fish[i] -> get_is_eaten())
                {
                    // delete it from the network.
                    delete_fish(fish[i]);
                    
                    // The last fish replaced the current fish and current_fish_amount decreased by 1. We want to check the last fish as well.
                    i = min(0, i - 1);
                }
            }
        }
        
        // Move all the fish in the network to their next step.
        void move()
        {           
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                // Move the fish and save the feedback.
                fish[i] -> move();

                // Check if the fish is out of bounds.
                if (fish[i] -> get_is_fish_out_of_bounds())
                {
                    // That's the end of the fish.
                    delete_fish(fish[i]);
                    
                    // The last fish replaced the current fish and current_fish_amount decreased by 1. We want to check the last fish as well.
                    i = min(0, i - 1);
                }
                
                else
                {
                    // Refresh the entity on the grid.
                    grid -> refresh_entity(*fish);
                }
            }
        }
        
        // Prepare the gifs of all the fish to their next frame.
        void set_next_frame()
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                fish[i] -> set_next_frame();
            }
        }
        
        // Draw the next frame of all the fish.
        void draw_next_frame()
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                fish[i] -> draw_next_frame();
            }
        }
        
        // The fish is gone, remove it from the network.
        void delete_fish(WanderFish* fish_to_remove)
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                // Check if this is the current fish.
                if (fish_to_remove == fish[i])
                {
                    // Replace the last fish to the current cell.
                    fish[i] = fish[current_fish_amount - 1];
                    
                    // Free the cell of the last fish, which was replaced.
                    current_fish_amount--;
                    
                    // That's it.
                    break;
                }
            }
            
            // Remove the fish from the grid.
            grid -> remove_entity(fish_to_remove);
        }
        
        void delete_network()
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                fish[i] -> delete_gif();
            }
        }
        
        // The function receives a fish profile and loads it to the fish network.
        void load_fish_profile(fish_profile current_fish_profile)
        {
            // Randomize a path stack from the paths_stack array of the fish profile.
            int random_paths_stack_index = rand() % current_fish_profile.paths_stacks_amount;
            
            // The cells within array of the new fish.
            Cell** cells_within = new Cell*[grid -> get_rows_amount() * grid -> get_columns_amount()];
            
            // Create the fish.
            WanderFish* fish_to_load = new WanderFish(current_fish_profile.file_path, current_fish_profile.fish_type, current_fish_profile.paths_stacks[random_paths_stack_index].initial_location, current_fish_profile.paths_stacks[random_paths_stack_index].is_left, current_fish_profile.size, current_fish_profile.min_speed_x, current_fish_profile.max_speed_x, current_fish_profile.min_speed_y, current_fish_profile.max_speed_y, current_fish_profile.min_frames_per_path, current_fish_profile.max_frames_per_path, current_fish_profile.paths_stacks[random_paths_stack_index], 0, grid -> get_width_pixels(), 0 + current_fish_profile.size.height, grid -> get_height_pixels() - current_fish_profile.size.height, 1, current_fish_profile.max_scaling, 0, current_fish_profile.is_facing_left_on_startup, grid -> get_rows_amount() * grid -> get_columns_amount(), cells_within);
            
            // Save the fish in the fish array.
            fish[current_fish_amount] = fish_to_load;
            current_fish_amount++;
            
            // Add the fish to the grid.
            grid -> add_entity(fish_to_load);
        }
};



// ----- Main Code -----


int main()
{
	// ----- Initialization -----
	
	// --- Constants ---
	
	// - Screen
	const int SCREEN_WIDTH = 1800;
	const int SCREEN_HEIGHT = 800;
	const char* SCREEN_TITLE = "The Fish";
	const int FPS = 30;
    
    // - Graphics Paths
    const char* PATH_MY_FISH = "Textures/my_fish.gif";
    const char* PATH_WORLD1 = "Textures/Worlds/world1.png";
    const char* PATH_FISH1 = "Textures/Fish/fish1.gif";
    
    // - Game Properties
    const int FISH_POPULATION = 3;
    const int GRID_ROWS = 1;
    const int GRID_COLS = 1;

	// --- GUI Initialization ---
	
	// Screen set-up.
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
	
	// Fps declaration.
	SetTargetFPS(FPS);
    
    // Load Textures.
    Texture2D world1 = LoadTexture(PATH_WORLD1);
    
    // Create the grid.
    Grid grid = Grid(GRID_COLS, GRID_ROWS, FISH_POPULATION, world1.width, world1.height);

    // --- Create Entities ---
    
    // - my fish.
    Cell** cells_within_my_fish = new Cell*[GRID_ROWS * GRID_COLS];
    
    MyFish my_fish = MyFish(PATH_MY_FISH, Location(world1.width / 2, world1.height / 2), Size(300, 300), 15, 12, 0, 0, 0, 0, 1, 30, 0, false, FISH_POPULATION, cells_within_my_fish);

    // -- Fish Network --
   
    // - Fish Profiles
    
    // Fish 1
    
    // Right
    fish_path fish1_path_wander_right = {10, 0, true, true, 100};
    fish_path fish1_wander_right_paths[] = {fish1_path_wander_right};
    paths_stack fish1_paths_stack_wander_right = {Location(), 1, fish1_wander_right_paths, false, false, true};
    
    // Left
    fish_path fish1_path_wander_left = {10, 0, false, true, 100};
    fish_path fish1_wander_left_paths[] = {fish1_path_wander_left};
    paths_stack fish1_paths_stack_wander_left = {Location(), 1, fish1_wander_left_paths, false, false, false};
    
    paths_stack fish1_paths_stacks[] = {fish1_paths_stack_wander_right, fish1_paths_stack_wander_left};
    fish_profile fish1 = {PATH_FISH1, "fish 1", true, Size(150, 150), 3, 4, 30, 0, 2, 30, 300, 2, fish1_paths_stacks, 1};
    
    // - set the fish network
    
    fish_profile fish_profiles_on_startup[] = {fish1, fish1};
    fish_profile available_fish[] = {fish1};
    FishNetwork fish_network = FishNetwork(FISH_POPULATION, &grid, fish_profiles_on_startup, 2, available_fish, 1);
    fish_network.setup();
    
    /*
    // Fish 1
    
    // Right
    fish_path fish1_path_wander_right = {10, 0, true, true, 100};
    fish_path fish1_wander_right_paths[] = {fish1_path_wander_right};
    paths_stack fish1_paths_stack_wander_right = {Location(), 1, fish1_wander_right_paths, true, false, true};
      
    paths_stack fish1_paths_stacks[] = {fish1_paths_stack_wander_right};
    fish_profile fish1 = {PATH_FISH1, "fish 1", true, Size(150, 150), 3, 4, 30, 0, 2, 30, 300, 1, fish1_paths_stacks, 1};
    
    // - set the fish network
    
    fish_profile fish_profiles_on_startup[] = {fish1};
    fish_profile available_fish[] = {fish1};
    FishNetwork fish_network = FishNetwork(FISH_POPULATION, &grid, fish_profiles_on_startup, 1, available_fish, 1);
    fish_network.setup();
    */
    
    
    
    // Add all the entities to the grid.
    grid.add_entity(&my_fish);

    // Create and setup the camera.
    Camera2D camera = { 0 };
    //camera.target = (Vector2){ my_fish.get_location().x, my_fish.get_location().y };
    //camera.offset = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    camera.rotation = 0;
    camera.zoom = 0.3;
    
	// ----- Game Loop -----
	
	// As long as the Esc button or exit button were not pressed, continue to the next frame.
	while (!WindowShouldClose())
	{
        // --- Boundaries Management ---
        
        // Set the relevant boundaries for all the fish (its scaling considartions occurs within the fish update boundaries calls).
        // Needs to be updated each frame becuase the scaling of the fish can be changed.
        
        // Update my fish.
        //my_fish.update_boundaries((SCREEN_WIDTH / camera.zoom) / 2, world1.width - (SCREEN_WIDTH / camera.zoom) / 2, (SCREEN_HEIGHT / camera.zoom) / 2, world1.height - (SCREEN_HEIGHT / camera.zoom) / 2, true);
        my_fish.update_boundaries(0, world1.width, 0, world1.height, true);
        // Update the fish network.
        fish_network.update_boundaries(0, world1.width, 0, world1.height, true);
        
        // --- User Input Management ---
        
        // Handle arrow keys strokes. They move the fish in the world.
        if (IsKeyDown(KEY_RIGHT)) { my_fish.move_right(); grid.refresh_entity(&my_fish); }
        if (IsKeyDown(KEY_LEFT)) { my_fish.move_left(); grid.refresh_entity(&my_fish); }
        if (IsKeyDown(KEY_UP)) { my_fish.move_up(); grid.refresh_entity(&my_fish); }
        if (IsKeyDown(KEY_DOWN)) { my_fish.move_down(); grid.refresh_entity(&my_fish); }
        
        // --- Entities Calculations ---
        
        // Remove all the eaten fish from the previous frame.
        fish_network.handle_eaten();
        
        // Release available fish.
        fish_network.load_available_fish();
        
        // Move all the fish in the fish network.
        fish_network.move();
        
        // --- Handle Collisions ---
        
        // A reference to the cells array.
        Cell*** grid_cells = grid.get_cells();
        
        // The amount of entities currently in the current cell.
        int current_cell_entities_amount;
        
        // The entities array of the current cell.
        GridEntity** entities_in_cell;
        
        // The rectangular frame of the two entities in the loop.
        Rectangle first_entity_rectangle;
        Rectangle second_entity_rectangle;    
        
        cout << "\nNew frame:\n";
        
        // Iterate over the cells of the grid.
        for (int row_index = 0; row_index < grid.get_rows_amount(); row_index++)
        {
            for (int col_index = 0; col_index < grid.get_columns_amount(); col_index++)
            {
                // Save the amount of entities in the current cell.
                current_cell_entities_amount = grid_cells[row_index][col_index] -> get_entities_counter();
                
                
                if (current_cell_entities_amount > 0)
                {
                    cout << "--> (" << row_index << ", " << col_index << ") " << current_cell_entities_amount << " entities.\n";
                }
                
                
                // Get the array of entities in the cell.
                entities_in_cell = grid_cells[row_index][col_index] -> get_entities();
                
                // Iterate over all the possible entities pairs in the current cell.
                for (int first_entity_index = 0; first_entity_index < current_cell_entities_amount; first_entity_index++)
                {
                    for (int second_entity_index = first_entity_index + 1; second_entity_index < current_cell_entities_amount; second_entity_index++)
                    {
                        // Get the rectangle frame of the two entities.
                        first_entity_rectangle = entities_in_cell[first_entity_index] -> get_updated_rectangular_frame();
                        second_entity_rectangle = entities_in_cell[second_entity_index] -> get_updated_rectangular_frame();
                        
                        // Check if the two current entities are overlapping.
                        if (CheckCollisionRecs(first_entity_rectangle, second_entity_rectangle))
                        {
                            // Tell the first entity it collided with the second entity.
                            entities_in_cell[first_entity_index] -> handle_collision(entities_in_cell[second_entity_index]);
                        }
                    }
                }
            }
        }
        
        cout << "End of frame.\n";
        
        // --- Camera ---
        
        // Camera follows my fish movement.
        //camera.target = (Vector2){ my_fish.get_location().x, my_fish.get_location().y };
        
        // --- Prepare Gifs for drawing ---
        
        // Prepare all the fish to their next gif frame.
		my_fish.set_next_frame();        
        fish_network.set_next_frame();        
        
        // --- Draw ---
        
        BeginDrawing();
            
            // Clear the background.
            ClearBackground(RAYWHITE);
            
            // Everything inside this scope, is being manipulated by the camera.
            // Every drawing outside this scope, will show up on the screen without being transformed by the camera.
            BeginMode2D(camera);
                
                // Draw the background.
                DrawTexture(world1, 0, 0, WHITE);

                // Draw the next gif frame of the fish.
                my_fish.draw_next_frame();
                fish_network.draw_next_frame();
            
            // The end of the drawings affected by the camera.
            EndMode2D();

        EndDrawing();
	}
	
	// ----- Close Game -----
	
    my_fish.delete_gif();
    fish_network.delete_network();
    
	// Close the game screen.
	CloseWindow();
}