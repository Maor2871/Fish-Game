#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <thread>
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
    
    // An initial y coordinate.
    int initial_y_coordinate;
    
    // How many paths in the stack.
    int length;
    
    // An array of the paths.
    fish_path* paths;
    
    // If true the path repeats itself.
    bool is_repeat;
    
    // Flags for the initial location.
    bool is_initial_location;
    bool is_initial_y_coordinate;
    bool is_randomize_x_coordinate;
    bool is_left;
};

// Fish recipe.
struct fish_profile
{
    // - Basic properties.
    
    Image fish_image;
    int* fish_image_frames_amount;
    string fish_type;
    bool is_sting_proof;
    bool is_facing_left_on_startup;
    Size size;
    float max_scaling;
    float min_speed_x;
    float max_speed_x;
    float min_speed_y;
    float max_speed_y;
    int min_frames_per_path;
    int max_frames_per_path;
    float can_eat_ratio;
    float cant_eat_ratio;
    bool is_randomize_initial_scale;
    
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
        
        // Clears the current cell.
        void reset()
        {
            entities_counter = 0;
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
        
        // Default Constructor.
        Grid()
        {
            cell_maximum_amount_of_entities = 0;
            columns_amount = 0;
            rows_amount = 0;
            width_pixels = 0;
            height_pixels = 0;
            cell_width_pixels = 0;
            cell_height_pixels = 0;
        }
        
        // Reset the grid.
        void reset() 
        {
            // Iterate over the cells in the grid.
            for (int row_index = 0; row_index < rows_amount; row_index++)
                for (int col_index = 0; col_index < columns_amount; col_index++)
                    cells[row_index][col_index] -> reset();
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
            int x_boundary_left = location.x - ((sqrt(scale) * size.width) / 2);
            int x_boundary_right = x_boundary_left + (sqrt(scale) * size.width);
            
            // Calculate the y boundaries.
            int y_boundary_top = location.y - ((sqrt(scale) * size.height) / 2);
            int y_boundary_bottom = y_boundary_top + (sqrt(scale) * size.height);
            
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
        
        // Getters.
        int get_width_pixels() { return width_pixels; }
        int get_height_pixels() { return height_pixels; }
        int get_cell_width_pixels() { return cell_width_pixels; }
        int get_cell_height_pixels() { return cell_height_pixels; }
        
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
    
        // An optional attribute, if true on intialization, the gif originaly facing left. Indicates to flip it horizontaly to face right on startup.
        bool is_facing_left_on_startup;
        
        // If true, flips the gif horizontally.
        bool is_flip_horizontal;
        
        // If true, flips the gif vertically.
        bool is_flip_vertical;
        
        // When the gif is loaded, this variable contains the amount of frames this gif has.
        int* frames_amount;
        
        // Pointing on the current frame of the gif that is being displayed.
        int current_frame = 0;
        
        // The gif as image.
        Image my_gif_image;

        // The gif as texture.
        Texture2D my_gif_texture;
        
        // The tint of the gif.
        Color tint;
    
    public:
    
        // Constructor.
        MyGif(Image new_my_gif_image, int* new_frames_amount, string new_entity_type, Location new_location, Size new_size, float new_scale, float new_max_scale, int new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : GridEntity(new_entity_type, new_location, new_size, new_scale, new_max_scale, new_rotation, new_max_cells_within, new_cells_within)
        {
            // Save the image of the gif.
            my_gif_image = new_my_gif_image;
            
            // Save the frames amount instance.
            frames_amount = new_frames_amount;
            
            // True if the gif faces left originaly.
            is_facing_left_on_startup = new_is_facing_left_on_startup;
            
            // Set the texture on initialization to face right.
            if (is_facing_left_on_startup) { is_flip_horizontal = true; }
            else { is_flip_horizontal = false; }

            // Do not flip the texture verticaly on initialization.
            is_flip_vertical = false;           

            // Create the texture instance.
            my_gif_texture = LoadTextureFromImage(my_gif_image);
            
            // Set the tint to white.
            tint = WHITE;
        }
        
        // Default Constructor.
        MyGif() : GridEntity("", Location(), Size(), 1, 1, 0, 1, new Cell*[0])
        {
            is_facing_left_on_startup = false;
            is_flip_horizontal = false;
            is_flip_vertical = false;
        }
        
        // Flipping manipulations
        void flip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = false : is_flip_horizontal = true;}
        void flip_vertical() {is_flip_vertical = true;}
        void unflip_horizontal() {is_facing_left_on_startup ? is_flip_horizontal = true : is_flip_horizontal = false;}
        void unflip_vertical() {is_flip_vertical = false;}
        
        // Change the tint of the gif.
        void update_tint(Color new_tint) { tint = new_tint; }
        
        // The function Prepare the next frame of the gif.
        void set_next_frame()
        {
            // Containing required data about the next frame.
            int next_frame_data_offset;
            
            // Point on the next frame.
            current_frame++;
            
            // Reset the current frame index if currently displaying the last frame of the gif.
            if (current_frame >= *frames_amount) { current_frame = 0; }

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
            Rectangle destination = {location.x, location.y, (int) floor(sqrt(scale) * size.width), (int) floor(sqrt(scale) * size.height)};
            
            // We want the gif to be rotated in relation to its center, and we want that the inputed location in the destination rectangle will be the center.
            Vector2 center = {(sqrt(scale) * size.width) / 2, (sqrt(scale) * size.height) / 2};
            
            // Draw the next frame of the gif properly.
            DrawTexturePro(my_gif_texture, source, destination, center, rotation, tint);
        }

        // The function removes the gif from the screen.
        void delete_gif()
        {
            // Remove the texture.
            UnloadTexture(my_gif_texture);
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
        
        // How many frames elapsed in a second.
        int fps;

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
        
        // How much the fish grows after eating another fish.
        float eat_grow_ratio;
        
        // Can eat fish that are <can_eat_ratio> my size and greater.
        float can_eat_ratio;
        
        // Can't eat fish that are <cant_eat_ratio> and less my size.
        float cant_eat_ratio;
        
        // If true the fish can't get stinged.
        bool is_sting_proof;
        
        // How many frames left for the current stunt to over.
        int current_stunt_frames_left;
        
        // How much size decreases each frame from the current stunt.
        float stunt_size_decrease_per_frame;

        // The original speed of the fish (for when the speed is temporarly changed).
        int original_speed_x;
        int original_speed_y; 
       
    public:
        
        // Counstructor.
        Fish(int new_fps, Image new_my_fish_image, int* new_frames_amount, string new_fish_type, bool new_is_sting_proof, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_max_scale, float new_eat_grow_ratio, float new_can_eat_ratio, float new_cant_eat_ratio, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within) : MyGif(new_my_fish_image, new_frames_amount, "Fish", new_location, new_size, new_scale, new_max_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
            // How many frames are there per second.
            fps = new_fps;
            
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
            
            // The ratio of eating and growing.
            eat_grow_ratio = new_eat_grow_ratio;
            
            // The ratio of fish size can eat.
            can_eat_ratio = new_can_eat_ratio;
            
            // The ratio of fish size can't eat.
            cant_eat_ratio = new_cant_eat_ratio;
            
            // Stunt properties.
            is_sting_proof = new_is_sting_proof;
            current_stunt_frames_left = 0;
            stunt_size_decrease_per_frame = 0;
        }

        // Default Constructor.
        Fish() : MyGif()
        {
            fish_type = "none";
            speed_x = 1;
            speed_y = 1;
            left_boundary = 0;
            right_boundary = 0;
            top_boundary = 1;
            bottom_boundary = 1;
            is_fish_out_of_bounds = false;
            is_eaten = false;
        }

        // Apply movements (including boundaries check).
        void move_left() 
        {
            if (location.x - speed_x < left_boundary) { location.x = left_boundary; boundary_exceed(); is_fish_out_of_bounds = true; }
            else { location.x -= speed_x; flip_horizontal(); }
        }
        
        void move_right() 
        {
            if (location.x + speed_x > right_boundary) { location.x = right_boundary; boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.x += speed_x; unflip_horizontal(); }
        }
        
        void move_up() 
        {
            if (location.y - speed_y < top_boundary) { location.y = top_boundary; boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.y -= speed_y; }
        }
        
        void move_down() 
        {
            if (location.y + speed_y > bottom_boundary) { location.y = bottom_boundary; boundary_exceed(); is_fish_out_of_bounds = true;}
            else { location.y += speed_y; }
        }
        
        // The function causes the fish a jelly fish stunt.
        void jelly_fish_stunt(float stunt_power)
        {
            // Sting proof.
            if (is_sting_proof) { return; }
            
            // Currently in a stunt, can't be stunt again.
            if (current_stunt_frames_left > 0) { return; }
            
            // Stunt.
            current_stunt_frames_left = (int) floor(stunt_power * fps);
            
            // Calculate how much scale decreases each frame.
            stunt_size_decrease_per_frame = (((scale - 1) / 2) * stunt_power) / current_stunt_frames_left;
            
            // Save the current speed and stunt.
            original_speed_x = speed_x;
            original_speed_y = speed_y;
            speed_x = 0;
            speed_y = 0;
        }
        
        // Update the stunt for the current frame.
        void update_stunt()
        {
            // If currently finished stun.
            if (current_stunt_frames_left == 1)
            {
                // If the scale is not big enough, the fish is dead.
                if (scale < 1.2) { eaten(); return; }
                
                // Restore the speed of the fish.
                speed_x = original_speed_x;
                speed_y = original_speed_y;
                update_tint(WHITE);
                current_stunt_frames_left--;
            }
            
            // Move to the next frame.
            if (current_stunt_frames_left > 0) 
            {
                current_stunt_frames_left--;
                
                // Update the tint for sting effect.
                if (current_stunt_frames_left % 2 == 1) { tint = BLUE; }
                else { tint = WHITE; }
                
                // Decrease the size.
                scale -= stunt_size_decrease_per_frame;
            }
        }
        
        // The function receives new boundaries, and sets them as the new boundaries of the fish.
        void update_boundaries(int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, bool is_consider_size)
        {
            // Note: the boundaries are compared to the current location of the fish, which considers its center. Therefore the precise boundary would be calculated with half the current dimentions of the fish.
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
        void boundary_exceed() { if (fish_type != "my fish") { delete_gif(); } }
        
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
        bool eat (int pixels)
        {
            // Cannot eat, already max size.
            if (scale == max_scale) { return false; }
            
            // Use the ratio to decide how much of the recieved pixels to digest.
            pixels = (int) floor(pixels * eat_grow_ratio);
            
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
            float new_scale = (float) pow(((size.width * sqrt(scale)) + full_loops_counter + ( (double) pixels / current_pixels_for_loop) ) / size.width, 2);
            if (new_scale > max_scale) { scale = max_scale; }
            else { scale = new_scale; }
            
            // Scaled up.
            return true;
        }
        
        // The function is being called when the fish is getting eaten.
        void eaten()
        {
            // Indicate that the fish is getting eaten.
            is_eaten = true;
        }
        
        // Eat ratio getters.
        float get_can_eat_ratio() { return can_eat_ratio; }
        float get_cant_eat_ratio() { return cant_eat_ratio; }
        
        // The function handles a collision between the fish and another GridEntity (Note that a collision between two entities is called only once).
        void handle_collision(GridEntity* collided_with_entity)
        {
            // It's a collision of two fish.
            if (collided_with_entity -> get_entity_type() == "Fish")
            {   
                // There is no canibalism in fish-hood.
                if (fish_type == ((Fish*) collided_with_entity) -> get_fish_type())
                {
                    // Ignore the collision.
                    return;
                }
                
                // - Handle My Fish and Jelly Fish Collision -
                
                if (((Fish*) collided_with_entity) -> get_fish_type().rfind("Jelly Fish", 0) == 0)
                {
                    jelly_fish_stunt(((Fish*) collided_with_entity) -> scale);
                    return;
                }
                
                else if (fish_type.rfind("Jelly Fish", 0) == 0)
                {
                    ((Fish*) collided_with_entity) -> jelly_fish_stunt(scale);
                    return;
                }
                
                // - Handle Classic Fish Collision -
                
                // The size in width of the current fish.
                int my_size = (int) floor(size.width * size.height * scale);
            
                // The size in width of the received fish.
                int other_size = (int) floor(collided_with_entity -> get_size().width * collided_with_entity -> get_size().height * collided_with_entity -> get_scale());

                // Check if the current fish can eat the other one.
                if (my_size > other_size * can_eat_ratio && (my_size < other_size * cant_eat_ratio || fish_type == "my fish" || ((Fish*) collided_with_entity) -> get_fish_type() == "my fish"))
                {
                    // Try to eat, if ate, update eaten.
                    if (eat(other_size)) { ((Fish*) collided_with_entity) -> eaten(); }
                }
                
                // Check if the other fish can eat the current one.
                else if (other_size > my_size * ((Fish*) collided_with_entity) -> get_can_eat_ratio() && (other_size < my_size * ((Fish*) collided_with_entity) -> get_cant_eat_ratio() || fish_type == "my fish" || ((Fish*) collided_with_entity) -> get_fish_type() == "my fish"))
                {
                    // Try to eat, if ate, update eaten.
                    if (((Fish*)collided_with_entity) -> eat(my_size)) { eaten(); }
                }
            }           
        }
};


class MyFish : public Fish
{
    /*
        Represents the fish of the user.
    */
    
    protected:
        
        // The speed multiplier when turbo is on.
        float turbo;
        
        // How many frames the turbo last.
        int turbo_duration_frames;
        
        // How many frames left till the end of the current turbo.
        int turbo_duration_frames_left;
        
        // How many frames last to finish the turbo reloading.
        int turbo_reload_frames_left;
        
        // How many frames it takes to revive the turbo.
        int turbo_reload_frames;
    
        // When my fish reach that scale, victory.
        float required_scale;
        
        // Where on the screen to draw the scale widget.
        Location scale_widget_location;
        
        // The size of the scale widget.
        Size scale_widget_size;
        
        // The stroke of the scale widget.
        int scale_widget_stroke;
        
        // Where on the screen to draw the scale widget.
        Location turbo_widget_location;
        
        // The size of the scale widget.
        Size turbo_widget_size;
        
        // The stroke of the scale widget.
        int turbo_widget_stroke;
        
        // Data relevant for resetting.
        Location startup_location;
        float original_scale;
        
    public:
    
        // Constructor.
        MyFish(int new_fps, Image new_my_fish_image, int* new_frames_amount, bool new_is_sting_proof, Location new_location, Size new_size, int new_speed_x, int new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_required_scale, float new_eat_grow_ratio, float new_can_eat_ratio, float new_cant_eat_ratio, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within, Cell** new_cells_within, Location new_scale_widget_location, Size new_scale_sidget_size, int new_scale_widget_stroke, float new_turbo, int new_turbo_duration_frames, int new_turbo_reload_frames, Location new_turbo_widget_location, Size new_turbo_widget_size, int new_turbo_widget_stroke) : Fish(new_fps, new_my_fish_image, new_frames_amount, "my fish", new_is_sting_proof, new_location, new_size, new_speed_x, new_speed_y, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_required_scale, new_eat_grow_ratio, new_can_eat_ratio, new_cant_eat_ratio, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
            // Initialize my fish properties.
            fps = new_fps;
            current_stunt_frames_left = 0;
            required_scale = new_required_scale;
            scale_widget_location.set_location(new_scale_widget_location);
            scale_widget_size = new_scale_sidget_size;
            scale_widget_stroke = new_scale_widget_stroke;
            turbo = new_turbo;
            turbo_duration_frames = new_turbo_duration_frames;
            turbo_duration_frames_left = 0;
            turbo_reload_frames = new_turbo_reload_frames;
            turbo_reload_frames_left = 0;
            turbo_widget_location = new_turbo_widget_location;
            turbo_widget_size = new_turbo_widget_size;
            turbo_widget_stroke = new_turbo_widget_stroke;
            
            // Properties relevant for resetting.
            startup_location = new_location;
            original_scale = new_scale;
        }
        
        // Default Constructor.
        MyFish() : Fish()
        {
            
        }
        
        // Resets the fish to its initial properties.
        void reset()
        {
            current_stunt_frames_left = 0;
            turbo_duration_frames_left = 0;
            turbo_reload_frames_left = 0;
            location.set_location(startup_location);
            scale = original_scale;
        }
        
        // Returns true if my fish has reached its required scale.
        bool is_victory() { return scale >= required_scale; }
        
        // Returns False if my fish got eaten and the game is over.
        bool is_alive() { return !is_eaten; }
        
        // Draws the current scale widget.
        void draw_scale_widget()
        {
            // Draw the scale title.
            DrawText("Scale", scale_widget_location.x - 75, scale_widget_location.y, 25, BLACK);
            
            // Draw the stroke rectangle.
            DrawRectangle(scale_widget_location.x - scale_widget_stroke, scale_widget_location.y - scale_widget_stroke, scale_widget_size.width + 2 * scale_widget_stroke, scale_widget_size.height + 2 * scale_widget_stroke, BLACK);
            
            // Draw the container rectangle.
            DrawRectangle(scale_widget_location.x, scale_widget_location.y, scale_widget_size.width, scale_widget_size.height, WHITE);
            
            // Draw the fill rectangle.
            DrawRectangle(scale_widget_location.x, scale_widget_location.y, scale_widget_size.width * (scale / required_scale), scale_widget_size.height, RED);
        }
        
        // If available, applies the turbo.
        void apply_turbo()
        {
            if (turbo_reload_frames_left == 0) 
            {
                turbo_reload_frames_left = turbo_reload_frames;
                turbo_duration_frames_left = turbo_duration_frames;
                speed_x *= turbo;
                speed_y *= turbo;
            }
        }
        
        // Sets another frame of turbo.
        void update_turbo()
        {
            if (turbo_reload_frames_left > 0) { turbo_reload_frames_left--; }
            if (turbo_duration_frames_left == 1) { speed_x /= turbo; speed_y /= turbo; }
            if (turbo_duration_frames_left > 0) { turbo_duration_frames_left--; }
        }
        
        void draw_turbo_widget()
        {
            // Draw the turbo title.
            DrawText("Turbo", turbo_widget_location.x - 75, turbo_widget_location.y, 25, BLACK);
            
            // Draw the turbo rectangle.
            DrawRectangle(turbo_widget_location.x - turbo_widget_stroke, turbo_widget_location.y - turbo_widget_stroke, turbo_widget_size.width + 2 * turbo_widget_stroke, turbo_widget_size.height + 2 * turbo_widget_stroke, BLACK);
            
            // Draw the container rectangle.
            DrawRectangle(turbo_widget_location.x, turbo_widget_location.y, turbo_widget_size.width, turbo_widget_size.height, WHITE);
            
            // Draw the fill rectangle.
            DrawRectangle(turbo_widget_location.x, turbo_widget_location.y, (int) (turbo_widget_size.width * ((float)(turbo_reload_frames - turbo_reload_frames_left) / turbo_reload_frames)), turbo_widget_size.height, RED);
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
        WanderFish(int new_fps, Image new_wander_fish_image, int* new_frames_amount, string new_fish_type, bool new_is_sting_proof, Location new_location, bool is_initial_left_location, bool is_randomize_x_coord, Size new_size, float new_min_speed_x, float new_max_speed_x, float new_min_speed_y, float new_max_speed_y, int new_min_path_frames, int new_max_path_frames, paths_stack new_paths_stack, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_max_scale, bool is_randomize_initial_scale, float new_eat_grow_ratio, float new_can_eat_ratio, float new_cant_eat_ratio, float new_rotation, bool new_is_facing_left_on_startup, int new_x_offset, int new_max_cells_within, Cell** new_cells_within) : Fish(new_fps, new_wander_fish_image, new_frames_amount, new_fish_type, new_is_sting_proof, new_location, new_size, 0, 0, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_max_scale, new_eat_grow_ratio, new_can_eat_ratio, new_cant_eat_ratio, new_rotation, new_is_facing_left_on_startup, new_max_cells_within, new_cells_within)
        {
            // Set the range of speeds on both axes.
            min_speed_x = new_min_speed_x;
            max_speed_x = new_max_speed_x;
            min_speed_y = new_min_speed_y;
            max_speed_y = new_max_speed_y;
            
            // Set the range of a new path frames.
            min_path_frames = new_min_path_frames;
            max_path_frames = new_max_path_frames;
            
            // If true, randomizing the initial scale.
            if (is_randomize_initial_scale) { scale = max(1, rand() % (int) floor(max_scale * 1000 - 1000) / 1000 + 1);  }
            
            // Set the paths stack, and save the number of elements in it.
            my_paths_stack = new_paths_stack;
            paths_stack_index = 0;
            
            // If there are paths in stack path, pull out the first one to be the current path.
            if (my_paths_stack.length > 0) 
            {
                // If initial location is relevant.
                if (my_paths_stack.is_initial_location) { location.set_location(my_paths_stack.initial_location); }
                
                // If initial y coordinate is set.
                else if (my_paths_stack.is_initial_y_coordinate) { set_random_initial_location(is_initial_left_location, my_paths_stack.is_randomize_x_coordinate || is_randomize_x_coord, new_x_offset, true, my_paths_stack.initial_y_coordinate); }
                
                // Otherwise, totally randomize the initial location.
                else { set_random_initial_location(is_initial_left_location, is_randomize_x_coord, new_x_offset, false, 0); }
                
                // Set the first path as the current path.
                current_path = my_paths_stack.paths[0];
                
                // Remember the original amount of frames.
                current_path_original_frames_amount = current_path.current_frames_left;
            }
            
            // Otherwise, generate new path and assign it.
            else 
            {
                current_path = generate_new_path();
                
                set_random_initial_location(is_initial_left_location, is_randomize_x_coord, new_x_offset, false, 0);
            }
            
            // Update the properties of the wander fish to match the properties of the current path.
            match_path_in_fish();
        }
        
        // Default Constructor.
        WanderFish() : Fish()
        {
            min_path_frames = 100;
            max_path_frames = 1000;
            paths_stack_index = 0;
            current_path_original_frames_amount = 1;
            min_speed_x = 1;
            max_speed_x = 10;
            min_speed_y = 1;
            max_speed_y = 1;
        }
        
        // The function sets a random initial_location.
        void set_random_initial_location(bool is_initial_left_location, bool is_randomize_x_coord, int x_coord_offset, bool is_initial_y_coordinate, int initial_y_coordinate)
        {
            int y_coordinates = 0;

            // Set the received y coordinate.
            if (is_initial_y_coordinate) { y_coordinates = initial_y_coordinate; }
            
            // Randomize the location of the y axis.
            else
            {
                if (bottom_boundary - top_boundary <= 0) { y_coordinates = top_boundary + size.height; }
                else { y_coordinates = rand() % (bottom_boundary - top_boundary) + top_boundary; }
            }

            // The x coordination.
            int x_coordinates = 0;
            
            // If random x coord is required.
            if (is_randomize_x_coord)
            {               
                // Randomize the x coordination.
                if (right_boundary - left_boundary <= 0) { x_coordinates = left_boundary; }
                else { x_coordinates = rand() % (right_boundary - left_boundary) + left_boundary; }
            }
            
            // Left or right x coordinate is required.
            else
            {
                // Randomize the x_coord_offset.
                x_coord_offset = rand() % (x_coord_offset);
                
                // The location should be at the left side of the world.
                if (is_initial_left_location) { x_coordinates = left_boundary + x_coord_offset; }
                
                // The location should be at the right side of the world.
                else { x_coordinates = right_boundary - x_coord_offset; }
            }
            
            // Set the initial location.
            location.set_location(Location(x_coordinates, y_coordinates));
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
        
        // How many fps there are per second.
        int fps;
        
        // The maximum fish population in the world.
        int max_population;
        
        // The current population.
        int current_population;
        
        // The eat grow ratio in the world.
        float eat_grow_ratio;
        
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
        
        // The x coordinate offset of new fish initialization.
        int x_coord_offset;
        
    public:

        // Constructor.
        FishNetwork(int new_fps, int new_max_population, float new_eat_grow_ratio, Grid* new_grid, fish_profile* new_fish_on_startup, int new_fish_on_startup_length, fish_profile* new_available_fish, int new_available_fish_length, int new_x_coord_offset)
        {
            // How many frames there are per second.
            fps = new_fps;
            
            // The max population.
            max_population = new_max_population;
            
            // Save the eat grow ratio in the world.
            eat_grow_ratio = new_eat_grow_ratio;
            
            // Set the x coordination offset.
            x_coord_offset = new_x_coord_offset;
            
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
        
        // Default Constructor.
        FishNetwork()
        {
            max_population = 1;
            current_population = 0;
            fish_on_startup = 0;
            available_fish_length = 0;
            current_fish_amount = 0;
            lot_range = 0;
        }
        
        // Reset the current fish network.
        void reset()
        {
            current_population = 0;
            current_fish_amount = 0;
        }
        
        // The function creates and loads all the fish on startup.
        void setup()
        {           
            // Load all the fish on startup.
            for (int i = 0; i < fish_on_startup_length; i++)
                load_fish_profile(fish_on_startup[i], true);
            
            // Add fish to the fish network, to fill the max fish population.
            load_available_fish(true);
        }
        
        // The function loads available fish up to the max fish population.
        void load_available_fish(bool is_on_setup)
        {
            // Keep loading fish up to the max population.
            while (current_fish_amount < max_population)
            {               
                // lot the next available fish.
                int random_lot = rand() % lot_range;
                
                // Current lots sum.
                int lots_sum = 0;
                
                // Spot the lotted fish, and load it.
                for (int i = 0; i < available_fish_length; i++)
                {
                    lots_sum += proportions_lot[i];
                    
                    // That's the fish to load.
                    if (random_lot < lots_sum)
                    {
                        // Load the lotted fish.
                        load_fish_profile(available_fish[i], is_on_setup);
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
                fish[i] -> update_boundaries(left, right, top, bottom - (int) ceil(sqrt(fish[i] -> get_scale()) * fish[i] -> get_size().height), is_consider_size);
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
                    i = max(0, i - 1);
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
                    i = max(0, i - 1);
                }
                
                else
                {
                    // Refresh the entity on the grid.
                    grid -> refresh_entity(fish[i]);
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
        void load_fish_profile(fish_profile current_fish_profile, bool is_on_setup)
        {
            // Randomize a path stack from the paths_stack array of the fish profile.
            int random_paths_stack_index = rand() % current_fish_profile.paths_stacks_amount;
            
            // The cells within array of the new fish.
            Cell** cells_within = new Cell*[grid -> get_rows_amount() * grid -> get_columns_amount()];
            
            // Create the fish.
            WanderFish* fish_to_load = new WanderFish(fps, current_fish_profile.fish_image, current_fish_profile.fish_image_frames_amount, current_fish_profile.fish_type, current_fish_profile.is_sting_proof, current_fish_profile.paths_stacks[random_paths_stack_index].initial_location, current_fish_profile.paths_stacks[random_paths_stack_index].is_left, is_on_setup, current_fish_profile.size, current_fish_profile.min_speed_x, current_fish_profile.max_speed_x, current_fish_profile.min_speed_y, current_fish_profile.max_speed_y, current_fish_profile.min_frames_per_path, current_fish_profile.max_frames_per_path, current_fish_profile.paths_stacks[random_paths_stack_index], - x_coord_offset, grid -> get_width_pixels() + x_coord_offset, 0 + current_fish_profile.size.height, grid -> get_height_pixels() - current_fish_profile.size.height, 1, current_fish_profile.max_scaling, current_fish_profile.is_randomize_initial_scale, eat_grow_ratio, current_fish_profile.can_eat_ratio, current_fish_profile.cant_eat_ratio, 0, current_fish_profile.is_facing_left_on_startup, x_coord_offset, grid -> get_rows_amount() * grid -> get_columns_amount(), cells_within);

            // Save the fish in the fish array.
            fish[current_fish_amount] = fish_to_load;
            current_fish_amount++;
            
            // Add the fish to the grid.
            grid -> add_entity(fish_to_load);
        }
            
        // For Debugging.
        void print_frames()
        {
            // Iterate over all the fish in the network.
            for (int i = 0; i < current_fish_amount; i++)
            {
                Rectangle frame = fish[i] -> get_updated_rectangular_frame();
                DrawRectangle(frame.x,frame.y, frame.width, frame.height, RED);
            }
        }
};

// ----- Technical Classes -----

class Save
{
    /*
        Reads the file with the game progress data. Attributes are public.
    */
    public:
        
        // The path to the saving file.
        string file_path;
        
        // The file handler.
        fstream save_file;
        
        // To what world the user has reached.
        int world_checkpoint;
        
        // The current content of the file.
        string current_file_content;
    
    public:
    
        Save(string new_file_path)
        {
            // Save the path of the save file.
            file_path = new_file_path;
            
            // Open the file.
            save_file.open(file_path, ios::in | ios::out | ios::app | ios::binary);
            
            // Default values.
            world_checkpoint = 1;
            
            // - Read the file -
            
            if (!save_file) { return; }
            
            // The current line read from the file.
            string current_line;
            
            // Do not read more than 100 lines from the file.
            int count = 100;
            
            // Iterate over the lines of the file.
            while (getline(save_file, current_line) && count > 0)
            {
                // Decrypt the current line.
                current_line = decrypt(current_line);

                // Add the line to the file content.
                current_file_content += current_line + "\n";

                // Analyze the line to get required data.
                
                // Read world checkpoint.
                if (current_line.rfind("world checkpoint: ", 0) == 0)
                {
                    int world_checkpoint_length = ((string) ("world checkpoint: ")).length();
                    if (current_line.length() >= world_checkpoint_length + 2 && isdigit(current_line[world_checkpoint_length]) && isdigit(current_line[world_checkpoint_length + 1])) 
                    {
                        string number = current_line.substr(world_checkpoint_length, 2);
                        world_checkpoint = stoi(number); 
                    }
                }
                
                // Count one more line.
                count--;
            }
            
            // Make the file ready to write.
            save_file.clear();
        }
        
        // The function updates the world checkpoint in the file and in the world_checkpoint variable.
        void update_world_checkpoint(int new_checkpoint)
        {
            // Update the value of world checkpoint.
            world_checkpoint = new_checkpoint;
            
            // Create the world checkpoint string.
            string world_checkpoint_string = "";
            if (world_checkpoint >= 10) { world_checkpoint_string = to_string(world_checkpoint); }
            else { world_checkpoint_string = "0" + to_string(world_checkpoint); }
            
            // Variables for the following loop.
            string current_line;
            int current_index = 0;
            int world_checkpoint_length = ((string) "world checkpoint: ").length();

            // Iterate over the content of the file.
            while (current_index < current_file_content.length())
            {
                // This is the end of the current line.
                if (current_file_content[current_index] == '\\' && current_index + 1 < (current_file_content.length()) && current_file_content[current_index + 1] == 'n') { current_line = ""; continue; }
                
                // Add the next character of the line.
                current_line += current_file_content[current_index];
                
                // Make the check.
                if (current_line.substr(0, world_checkpoint_length) == "world checkpoint: " && current_line.length() == world_checkpoint_length + 2)
                {
                    current_file_content[current_index - 1] = world_checkpoint_string[0];
                    current_file_content[current_index] = world_checkpoint_string[1];
                    break;
                }
                
                // Move to the next index.
                current_index++;
            }
            
            // Update the new content of the file.
            update_file_content();
        }
        
        // The function rewrites the file with the current file content value.
        void update_file_content()
        {
            // Close the file to write its new content.
            save_file.close();
            
            // Open the file and clear its current content.
            fstream save_file(file_path, ios::out | ios::trunc | ios::binary);
            
            // Write the updated content.
            save_file << encrypt(current_file_content);
            
            // Close the file.
            save_file.close();
            
            // And open it for reading again.
            save_file.open(file_path, ios::in | ios::out | ios::app | ios::binary);
        }
        
        // The function receives a string and encrypts it.
        string encrypt(string to_encrypt)
        {
            // The final encrypted string.
            string encrypted = "";
            
            // Iterate over the received string.
            for (int i = 0; i < to_encrypt.length(); i++)
            {
                // Keep on the lines structure.
                if (to_encrypt[i] == '\\' && i + 1 < (to_encrypt.length()) && to_encrypt[i + 1] == 'n') { to_encrypt[i] = '\\'; to_encrypt[i + 1] = 'n'; i += 1; continue; }

                // Encrypt the current char.
                encrypted += to_string(int(to_encrypt[i]) * (i + 4) + 7) + "_";
            }
            
            // Return the new encrypted string.
            return encrypted;
        }
        
        // The function recieves an encrypted string, decrypts it and returns the decrypted string.
        string decrypt(string to_decrypt)
        {
            // The decrypted string will be here at the end.
            string decrypted = "";
            
            // the current decrypted character.
            string current = "";
            
            // The index of the received string.
            int index = 0;
            
            // The current char index.
            int char_index = 0;

            // Iterate over the received string.
            while (index < to_decrypt.length())
            {
                // Is the current decrypted character fully loaded?
                if (to_decrypt[index] == '_')
                {
                    // Decrypt the current char (get it's ascii value).
                    decrypted += char((stoi(current) - 7) / (char_index + 4));
                    
                    // Reset the current decrypted char.
                    current = "";
                    
                    // Count another decrypted character.
                    char_index++;
                }
                
                // Keep loading the current decrypted character.
                else 
                {
                    // This is not a number, the received string was not encrypted according to the protocol. Return an empty string.
                    if (!isdigit(to_decrypt[index])) { return ""; }
                    
                    // Update the current decrypted character.
                    current += to_decrypt[index]; 
                }
                
                // Move to the next character in the received string.
                index++;
            }
            
            // Return the decrypted string.
            return decrypted;
        }
        
        // Close the game progress file.
        void quit()
        {
            save_file.close();
        }
};

// ----- Main Code -----


int main()
{
	// ##### --- Initialization --- #####
	
	// ### --- Constants --- ###
	
	// - Screen
	int SCREEN_WIDTH = 1000;
	int SCREEN_HEIGHT = 800;
	const char* SCREEN_TITLE = "The Fish";
	const int FPS = 30;
    
    // - Graphics Paths
    const char* PATH_MAIN_MENU = "Textures/Menus/Main Menu/Main Menu.png";
    const char* PATH_CAMPAIN_BUTTON = "Textures/Menus/Main Menu/Campain button.png";
    const char* PATH_MAP = "Textures/Menus/Map/Map.png";
    const char* PATH_VICTORY = "Textures/Menus/Windows/Victory.png";
    const char* PATH_DEFEAT = "Textures/Menus/Windows/Defeat.png";
    const char* PATH_MAP_BUTTON = "Textures/Menus/Windows/Map Button.png";
    const char* PATH_WORLD1_BUTTON = "Textures/Menus/Map/World 1 Button.png";
    const char* PATH_WORLD2_BUTTON = "Textures/Menus/Map/World 2 Button.png";
    const char* PATH_WORLD3_BUTTON = "Textures/Menus/Map/World 3 Button.png";
    const char* PATH_MY_FISH = "Textures/Fish/My Fish/My Fish.gif";
    const char* PATH_WORLD1 = "Textures/Worlds/World 1/World 1.png";
    const char* PATH_WORLD2 = "Textures/Worlds/World 2/World 2.png";
    const char* PATH_WORLD3 = "Textures/Worlds/World 3/World 3.png";
    const char* PATH_FISH1 = "Textures/Fish/Fish 1/fish 1.gif";
    const char* PATH_FISH2 = "Textures/Fish/Fish 2/fish 2.gif";
    const char* PATH_FISH3 = "Textures/Fish/Fish 3/fish 3.gif";
    const char* PATH_FISH4 = "Textures/Fish/Fish 4/fish 4.gif";
    const char* PATH_FISH5 = "Textures/Fish/Fish 5/fish 5.gif";
    const char* PATH_FISH6 = "Textures/Fish/Fish 6/fish 6.gif";
    const char* PATH_FISH7 = "Textures/Fish/Fish 7/fish 7.gif";
    const char* PATH_FISH8 = "Textures/Fish/Fish 8/fish 8.gif";
    const char* PATH_FISH9 = "Textures/Fish/Fish 9/fish 9.gif";
    const char* PATH_FISH10 = "Textures/Fish/Fish 10/fish 10.gif";
    const char* PATH_FISH11 = "Textures/Fish/Fish 11/fish 11.gif";
    const char* PATH_CRAB1 = "Textures/Crabs/Crab 1/Crab 1.gif";
    const char* PATH_CRAB2 = "Textures/Crabs/Crab 2/Crab 2.gif";
    const char* PATH_JEFLLY_FISH1 = "Textures/Jelly Fish/Jelly Fish 1/Jelly Fish 1.gif";
    const char* PATH_JEFLLY_FISH2 = "Textures/Jelly Fish/Jelly Fish 2/Jelly Fish 2.gif";
    
    // - Audio Paths
    const char* PATH_MUSIC_MAIN_THEME = "Music/Main Theme.mp3";
    const char* PATH_MUSIC_WORLD1 = "Music/World 1.mp3";
    const char* PATH_MUSIC_WORLD2 = "Music/World 2.mp3";
    const char* PATH_MUSIC_WORLD3 = "Music/World 3.mp3";
    
    // - Other Paths
    string path_game_progress_file = "./save.txt";
    
    // - Game Properties
    const int FISH_POPULATION = 50;
    const int GRID_ROWS = 3;
    const int GRID_COLS = 8;
    float EAT_GROW_RATIO = 0.5;
    const int X_COORD_OFFSET = 1000;
    bool debug = false;
    bool debug_camera = false;
    
    // Load game progress data.
    Save game_save = Save(path_game_progress_file);

	// ### --- GUI Initialization --- ###
	
	// Screen set-up.
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    
    // Audio set-up.
    InitAudioDevice();
	
	// Fps declaration.
	SetTargetFPS(FPS);
    
    // Set to full screen and save dimensions in SCREEN_WIDTH, SCREEN_HEIGHT.
    int monitor = GetCurrentMonitor();
    SCREEN_WIDTH = GetMonitorWidth(monitor);
    SCREEN_HEIGHT = GetMonitorHeight(monitor);
    SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    ToggleFullscreen();
    
    // Defines the current screen in the main loop.
    string current_screen = "Main Menu";
    
    // The current world.
    int current_world = 1;
    
    // # ----- Load images ----- #
    
    int my_fish_image_frames_amount;
    Image my_fish_image = LoadImageAnim(PATH_MY_FISH, &my_fish_image_frames_amount);
    
    int fish1_image_frames_amount;
    Image fish1_image = LoadImageAnim(PATH_FISH1, &fish1_image_frames_amount);
    
    int fish2_image_frames_amount;
    Image fish2_image = LoadImageAnim(PATH_FISH2, &fish2_image_frames_amount);
    
    int fish3_image_frames_amount;
    Image fish3_image = LoadImageAnim(PATH_FISH3, &fish3_image_frames_amount);
    
    int fish4_image_frames_amount;
    Image fish4_image = LoadImageAnim(PATH_FISH4, &fish4_image_frames_amount);
    
    int fish5_image_frames_amount;
    Image fish5_image = LoadImageAnim(PATH_FISH5, &fish5_image_frames_amount);
    
    int fish6_image_frames_amount;
    Image fish6_image = LoadImageAnim(PATH_FISH6, &fish6_image_frames_amount);
    
    int fish7_image_frames_amount;
    Image fish7_image = LoadImageAnim(PATH_FISH7, &fish7_image_frames_amount);
    
    int fish8_image_frames_amount;
    Image fish8_image = LoadImageAnim(PATH_FISH8, &fish8_image_frames_amount);
    
    int fish9_image_frames_amount;
    Image fish9_image = LoadImageAnim(PATH_FISH9, &fish9_image_frames_amount);
    
    int fish10_image_frames_amount;
    Image fish10_image = LoadImageAnim(PATH_FISH10, &fish10_image_frames_amount);
    
    int fish11_image_frames_amount;
    Image fish11_image = LoadImageAnim(PATH_FISH11, &fish11_image_frames_amount);
    
    int crab1_image_frames_amount;
    Image crab1_image = LoadImageAnim(PATH_CRAB1, &crab1_image_frames_amount);
    
    int crab2_image_frames_amount;
    Image crab2_image = LoadImageAnim(PATH_CRAB2, &crab2_image_frames_amount);
    
    int jeflly_fish1_image_frames_amount;
    Image jelly_fish1_image = LoadImageAnim(PATH_JEFLLY_FISH1, &jeflly_fish1_image_frames_amount);
    
    int jeflly_fish2_image_frames_amount;
    Image jelly_fish2_image = LoadImageAnim(PATH_JEFLLY_FISH2, &jeflly_fish2_image_frames_amount);
    
    // # ----- Load Audio ----- #
    
    Sound music_main_theme = LoadSound(PATH_MUSIC_MAIN_THEME);
    
    Sound music_world1 = LoadSound(PATH_MUSIC_WORLD1);
    
    Sound music_world2 = LoadSound(PATH_MUSIC_WORLD2);
    
    Sound music_world3 = LoadSound(PATH_MUSIC_WORLD3);
    
    Sound current_music;
    
    // # ----- Variables -----

	Texture2D world;
    MyFish my_fish;
    FishNetwork fish_network;
    Grid grid;
    Camera2D camera;
    int camera_pos_x = 0, camera_pos_y = 0;
    int current_cell_entities_amount;
    Cell*** grid_cells;
	int my_fish_current_width, my_fish_current_height;
    int camera_current_height = 0, camera_current_width = 0; 
    
    // # ----- Main Menu ----- #
    
    // Load the main menu texture.
    Texture2D main_menu = LoadTexture(PATH_MAIN_MENU);
    
    // Load the campain button.
    Texture2D campain_button = LoadTexture(PATH_CAMPAIN_BUTTON); 
    
    // Define frame rectangle for drawing.
    Rectangle campain_button_frame = { (int) floor (SCREEN_WIDTH / 2 - campain_button.width / 2), (int) floor(SCREEN_HEIGHT / 2 - main_menu.height / 2) + 300, campain_button.width, campain_button.height };
    
    // The current mouse point location.
    Vector2 mouse_point = { 0, 0 };
    
    // # ----- Map ----- #
    
    // Load the map texture.
    Texture2D map = LoadTexture(PATH_MAP);
    
    // Load the worlds buttons.
    Texture2D world1_button = LoadTexture(PATH_WORLD1_BUTTON);
    Texture2D world2_button = LoadTexture(PATH_WORLD2_BUTTON);
    Texture2D world3_button = LoadTexture(PATH_WORLD3_BUTTON);
    
    // Define frame rectangle for drawing.
    Rectangle world1_button_frame = { 100, 100, world1_button.width, world1_button.height };
    Rectangle world2_button_frame = { 250, 150, world2_button.width, world2_button.height };
    Rectangle world3_button_frame = { 400, 225, world3_button.width, world3_button.height };
    
    // # ----- Windows ----- #
    
    // The victory window.
    Texture2D victory = LoadTexture(PATH_VICTORY);
    
    // The defeat window.
    Texture2D defeat = LoadTexture(PATH_DEFEAT);
    
    // The back to map button.
    Texture2D back_to_map_button = LoadTexture(PATH_MAP_BUTTON);
    
    // The frame of the back to map button.
    Rectangle back_to_map_button_frame = {(int) floor(SCREEN_WIDTH / 2 - back_to_map_button.width / 2), (int) floor(SCREEN_HEIGHT / 2) + 150, back_to_map_button.width, back_to_map_button.height};
    
    // True if victory/defeat.
    bool is_victory = false, is_defeat = false;
    
    // # ----- World 1 ----- #
    
    // Load Textures.
    Texture2D world1 = LoadTexture(PATH_WORLD1);
    
    // Create the grid.
    Grid world1_grid = Grid(GRID_COLS, GRID_ROWS, FISH_POPULATION, world1.width, world1.height);

    // ----- Create Entities -----
    
    // --- my fish ---
    
    Cell** world1_cells_within_my_fish = new Cell*[GRID_ROWS * GRID_COLS];
    MyFish world1_my_fish = MyFish(FPS, my_fish_image, &my_fish_image_frames_amount, false, Location(world1.width / 2, world1.height / 2), Size(150, 107), 15, 12, 0, 0, 0, 0, 1, 15, EAT_GROW_RATIO, 1.2, 10000, 0, true, FISH_POPULATION, world1_cells_within_my_fish, Location(100, SCREEN_HEIGHT - 75), Size(150, 20), 1, 2, FPS * 2, FPS * 5, Location(350, SCREEN_HEIGHT - 75), Size(150, 20), 1);

    // --- Fish Network ---
   
    // -- Fish Profiles --
    
    // - Fish 1 -
    
    // Right.
    fish_path world1_fish1_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish1_wander_right_paths[] = {world1_fish1_path_wander_right};
    paths_stack world1_fish1_paths_stack_wander_right = {Location(), 0, 1, world1_fish1_wander_right_paths, false, false, false, false, true};
    
    // Left.
    fish_path world1_fish1_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish1_wander_left_paths[] = {world1_fish1_path_wander_left};
    paths_stack world1_fish1_paths_stack_wander_left = {Location(), 0, 1, world1_fish1_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish1_paths_stacks[] = {world1_fish1_paths_stack_wander_right, world1_fish1_paths_stack_wander_left};
    fish_profile world1_fish1 = {fish1_image, &fish1_image_frames_amount, "fish 1", false, true, Size(130, 73), 1.5, 4, 20, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish1_paths_stacks, 1};
    
    // - Fish 2 -
    
    // Right
    fish_path world1_fish2_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish2_wander_right_paths[] = {world1_fish2_path_wander_right};
    paths_stack world1_fish2_paths_stack_wander_right = {Location(), 0, 1, world1_fish2_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish2_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish2_wander_left_paths[] = {world1_fish2_path_wander_left};
    paths_stack world1_fish2_paths_stack_wander_left = {Location(), 0, 1, world1_fish2_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish2_paths_stacks[] = {world1_fish2_paths_stack_wander_right, world1_fish2_paths_stack_wander_left};
    fish_profile world1_fish2 = {fish2_image, &fish2_image_frames_amount, "fish 2", false, false, Size(130, 112), 1.75, 6, 15, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish2_paths_stacks, 0.6};
    
    // - Fish 3 -
    
    // Right
    fish_path world1_fish3_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish3_wander_right_paths[] = {world1_fish3_path_wander_right};
    paths_stack world1_fish3_paths_stack_wander_right = {Location(), 0, 1, world1_fish3_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish3_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish3_wander_left_paths[] = {world1_fish3_path_wander_left};
    paths_stack world1_fish3_paths_stack_wander_left = {Location(), 0, 1, world1_fish3_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish3_paths_stacks[] = {world1_fish3_paths_stack_wander_right, world1_fish3_paths_stack_wander_left};
    fish_profile world1_fish3 = {fish3_image, &fish3_image_frames_amount, "fish 3", false, true, Size(170, 150), 3, 3, 10, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish3_paths_stacks, 1};
    
    // - Fish 4 -
    
    // Right
    fish_path world1_fish4_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish4_wander_right_paths[] = {world1_fish4_path_wander_right};
    paths_stack world1_fish4_paths_stack_wander_right = {Location(), 0, 1, world1_fish4_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish4_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish4_wander_left_paths[] = {world1_fish4_path_wander_left};
    paths_stack world1_fish4_paths_stack_wander_left = {Location(), 0, 1, world1_fish4_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish4_paths_stacks[] = {world1_fish4_paths_stack_wander_right, world1_fish4_paths_stack_wander_left};
    fish_profile world1_fish4 = {fish4_image, &fish4_image_frames_amount, "fish 4", true, true, Size(90, 100), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world1_fish4_paths_stacks, 1.5};
    
    // - Fish 5 -
    
    // Right
    fish_path world1_fish5_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish5_wander_right_paths[] = {world1_fish5_path_wander_right};
    paths_stack world1_fish5_paths_stack_wander_right = {Location(), 0, 1, world1_fish5_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish5_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish5_wander_left_paths[] = {world1_fish5_path_wander_left};
    paths_stack world1_fish5_paths_stack_wander_left = {Location(), 0, 1, world1_fish5_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish5_paths_stacks[] = {world1_fish5_paths_stack_wander_right, world1_fish5_paths_stack_wander_left};
    fish_profile world1_fish5 = {fish5_image, &fish5_image_frames_amount, "fish 5", false, true, Size(130, 94), 1.5, 6, 18, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish5_paths_stacks, 0.1};
    
    // - Fish 6 -
    
    // Right
    fish_path world1_fish6_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish6_wander_right_paths[] = {world1_fish6_path_wander_right};
    paths_stack world1_fish6_paths_stack_wander_right = {Location(), 0, 1, world1_fish6_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish6_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish6_wander_left_paths[] = {world1_fish6_path_wander_left};
    paths_stack world1_fish6_paths_stack_wander_left = {Location(), 0, 1, world1_fish6_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish6_paths_stacks[] = {world1_fish6_paths_stack_wander_right, world1_fish6_paths_stack_wander_left};
    fish_profile world1_fish6 = {fish6_image, &fish6_image_frames_amount, "fish 6", true, false, Size(150, 122), 1.75, 6, 18, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish6_paths_stacks, 0.1};
    
    // - Fish 7 -
    
    // Right
    fish_path world1_fish7_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish7_wander_right_paths[] = {world1_fish7_path_wander_right};
    paths_stack world1_fish7_paths_stack_wander_right = {Location(), 0, 1, world1_fish7_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish7_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish7_wander_left_paths[] = {world1_fish7_path_wander_left};
    paths_stack world1_fish7_paths_stack_wander_left = {Location(), 0, 1, world1_fish7_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish7_paths_stacks[] = {world1_fish7_paths_stack_wander_right, world1_fish7_paths_stack_wander_left};
    fish_profile world1_fish7 = {fish7_image, &fish7_image_frames_amount, "fish 7", false, true, Size(250, 202), 1.75, 4, 10, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish7_paths_stacks, 0.1};
    
    // - Fish 8 -
    
    // Right
    fish_path world1_fish8_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish8_wander_right_paths[] = {world1_fish8_path_wander_right};
    paths_stack world1_fish8_paths_stack_wander_right = {Location(), 0, 1, world1_fish8_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish8_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish8_wander_left_paths[] = {world1_fish8_path_wander_left};
    paths_stack world1_fish8_paths_stack_wander_left = {Location(), 0, 1, world1_fish8_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish8_paths_stacks[] = {world1_fish8_paths_stack_wander_right, world1_fish8_paths_stack_wander_left};
    fish_profile world1_fish8 = {fish8_image, &fish8_image_frames_amount, "fish 8", true, true, Size(90, 81), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world1_fish8_paths_stacks, 1.5};
    
    // - Fish 9 -
    
    // Right
    fish_path world1_fish9_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish9_wander_right_paths[] = {world1_fish9_path_wander_right};
    paths_stack world1_fish9_paths_stack_wander_right = {Location(), 0, 1, world1_fish9_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish9_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish9_wander_left_paths[] = {world1_fish9_path_wander_left};
    paths_stack world1_fish9_paths_stack_wander_left = {Location(), 0, 1, world1_fish9_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish9_paths_stacks[] = {world1_fish9_paths_stack_wander_right, world1_fish9_paths_stack_wander_left};
    fish_profile world1_fish9 = {fish9_image, &fish9_image_frames_amount, "fish 9", false, true, Size(150, 123), 3, 6, 13, 0, 2, 30, 300, 1.2, 2, true, 2, world1_fish9_paths_stacks, 0.2};
    
    // - Fish 10 -
    
    // Right
    fish_path world1_fish10_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish10_wander_right_paths[] = {world1_fish10_path_wander_right};
    paths_stack world1_fish10_paths_stack_wander_right = {Location(), 0, 1, world1_fish10_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish10_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish10_wander_left_paths[] = {world1_fish10_path_wander_left};
    paths_stack world1_fish10_paths_stack_wander_left = {Location(), 0, 1, world1_fish10_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish10_paths_stacks[] = {world1_fish10_paths_stack_wander_right, world1_fish10_paths_stack_wander_left};
    fish_profile world1_fish10 = {fish10_image, &fish10_image_frames_amount, "fish 10", true, false, Size(300, 287), 2, 1, 6, 0, 2, 30, 300, 1.2, 2, false, 2, world1_fish10_paths_stacks, 0.05};
    
    // - Fish 11 -
    
    // Right
    fish_path world1_fish11_path_wander_right = {10, 0, true, true, 100};
    fish_path world1_fish11_wander_right_paths[] = {world1_fish11_path_wander_right};
    paths_stack world1_fish11_paths_stack_wander_right = {Location(), 0, 1, world1_fish11_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world1_fish11_path_wander_left = {10, 0, false, true, 100};
    fish_path world1_fish11_wander_left_paths[] = {world1_fish11_path_wander_left};
    paths_stack world1_fish11_paths_stack_wander_left = {Location(), 0, 1, world1_fish11_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world1_fish11_paths_stacks[] = {world1_fish11_paths_stack_wander_right, world1_fish11_paths_stack_wander_left};
    fish_profile world1_fish11 = {fish11_image, &fish11_image_frames_amount, "fish 11", false, true, Size(300, 255), 2, 1, 6, 0, 2, 30, 300, 1.2, 2, false, 2, world1_fish11_paths_stacks, 0.05};
    
    // -- Setup --
    
    fish_profile world1_fish_profiles_on_startup[] = {};
    fish_profile world1_available_fish[] = {world1_fish1, world1_fish2, world1_fish3, world1_fish4, world1_fish5, world1_fish6, world1_fish7, world1_fish8, world1_fish9, world1_fish10, world1_fish11};
    FishNetwork world1_fish_network = FishNetwork(FPS, FISH_POPULATION, EAT_GROW_RATIO, &world1_grid, world1_fish_profiles_on_startup, 0, world1_available_fish, 11, X_COORD_OFFSET);
    world1_fish_network.update_boundaries(-X_COORD_OFFSET, world1.width + X_COORD_OFFSET, 0, world1.height, true);

    // ----- Final Set-ups World1 -----

    // Create and set-up the camera.
    Camera2D world1_camera = { 0 };
    world1_camera.target = (Vector2) { world1_my_fish.get_location().x, world1_my_fish.get_location().y };
    world1_camera.offset = (Vector2) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    world1_camera.rotation = 0;
    
    if (debug_camera) { world1_camera.zoom = 0.15; }
    else { world1_camera.zoom = 0.7; }
    
    // # ----- World 2 ----- #
    
    // Load Textures.
    Texture2D world2 = LoadTexture(PATH_WORLD2);
    
    // Create the grid.
    Grid world2_grid = Grid(GRID_COLS, GRID_ROWS, FISH_POPULATION, world2.width, world2.height);

    // ----- Create Entities -----
    
    // --- my fish ---
    
    Cell** world2_cells_within_my_fish = new Cell*[GRID_ROWS * GRID_COLS];
    MyFish world2_my_fish = MyFish(FPS, my_fish_image, &my_fish_image_frames_amount, false, Location(world2.width / 2, world2.height / 2), Size(150, 107), 15, 12, 0, 0, 0, 0, 1, 15, EAT_GROW_RATIO, 1.2, 10000, 0, true, FISH_POPULATION, world2_cells_within_my_fish, Location(100, SCREEN_HEIGHT - 75), Size(150, 20), 1, 2, FPS * 2, FPS * 5, Location(350, SCREEN_HEIGHT - 75), Size(150, 20), 1);

    // --- Fish Network ---
   
    // -- Fish Profiles --
    
    // - Fish 1 -
    
    // Right.
    fish_path world2_fish1_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish1_wander_right_paths[] = {world2_fish1_path_wander_right};
    paths_stack world2_fish1_paths_stack_wander_right = {Location(), 0, 1, world2_fish1_wander_right_paths, false, false, false, false, true};
    
    // Left.
    fish_path world2_fish1_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish1_wander_left_paths[] = {world2_fish1_path_wander_left};
    paths_stack world2_fish1_paths_stack_wander_left = {Location(), 0, 1, world2_fish1_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish1_paths_stacks[] = {world2_fish1_paths_stack_wander_right, world2_fish1_paths_stack_wander_left};
    fish_profile world2_fish1 = {fish1_image, &fish1_image_frames_amount, "fish 1", false, true, Size(130, 73), 1.5, 4, 20, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish1_paths_stacks, 0.8};
    
    // - Fish 2 -
    
    // Right
    fish_path world2_fish2_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish2_wander_right_paths[] = {world2_fish2_path_wander_right};
    paths_stack world2_fish2_paths_stack_wander_right = {Location(), 0, 1, world2_fish2_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish2_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish2_wander_left_paths[] = {world2_fish2_path_wander_left};
    paths_stack world2_fish2_paths_stack_wander_left = {Location(), 0, 1, world2_fish2_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish2_paths_stacks[] = {world2_fish2_paths_stack_wander_right, world2_fish2_paths_stack_wander_left};
    fish_profile world2_fish2 = {fish2_image, &fish2_image_frames_amount, "fish 2", false, false, Size(130, 112), 1.75, 6, 15, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish2_paths_stacks, 0.8};
    
    // - Fish 3 -
    
    // Right
    fish_path world2_fish3_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish3_wander_right_paths[] = {world2_fish3_path_wander_right};
    paths_stack world2_fish3_paths_stack_wander_right = {Location(), 0, 1, world2_fish3_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish3_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish3_wander_left_paths[] = {world2_fish3_path_wander_left};
    paths_stack world2_fish3_paths_stack_wander_left = {Location(), 0, 1, world2_fish3_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish3_paths_stacks[] = {world2_fish3_paths_stack_wander_right, world2_fish3_paths_stack_wander_left};
    fish_profile world2_fish3 = {fish3_image, &fish3_image_frames_amount, "fish 3", false, true, Size(170, 150), 3, 3, 10, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish3_paths_stacks, 1};
    
    // - Fish 4 -
    
    // Right
    fish_path world2_fish4_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish4_wander_right_paths[] = {world2_fish4_path_wander_right};
    paths_stack world2_fish4_paths_stack_wander_right = {Location(), 0, 1, world2_fish4_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish4_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish4_wander_left_paths[] = {world2_fish4_path_wander_left};
    paths_stack world2_fish4_paths_stack_wander_left = {Location(), 0, 1, world2_fish4_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish4_paths_stacks[] = {world2_fish4_paths_stack_wander_right, world2_fish4_paths_stack_wander_left};
    fish_profile world2_fish4 = {fish4_image, &fish4_image_frames_amount, "fish 4", true, true, Size(90, 100), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world2_fish4_paths_stacks, 1};
    
    // - Fish 5 -
    
    // Right
    fish_path world2_fish5_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish5_wander_right_paths[] = {world2_fish5_path_wander_right};
    paths_stack world2_fish5_paths_stack_wander_right = {Location(), 0, 1, world2_fish5_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish5_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish5_wander_left_paths[] = {world2_fish5_path_wander_left};
    paths_stack world2_fish5_paths_stack_wander_left = {Location(), 0, 1, world2_fish5_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish5_paths_stacks[] = {world2_fish5_paths_stack_wander_right, world2_fish5_paths_stack_wander_left};
    fish_profile world2_fish5 = {fish5_image, &fish5_image_frames_amount, "fish 5", false, true, Size(130, 94), 1.5, 6, 18, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish5_paths_stacks, 0.8};
    
    // - Fish 6 -
    
    // Right
    fish_path world2_fish6_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish6_wander_right_paths[] = {world2_fish6_path_wander_right};
    paths_stack world2_fish6_paths_stack_wander_right = {Location(), 0, 1, world2_fish6_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish6_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish6_wander_left_paths[] = {world2_fish6_path_wander_left};
    paths_stack world2_fish6_paths_stack_wander_left = {Location(), 0, 1, world2_fish6_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish6_paths_stacks[] = {world2_fish6_paths_stack_wander_right, world2_fish6_paths_stack_wander_left};
    fish_profile world2_fish6 = {fish6_image, &fish6_image_frames_amount, "fish 6", true, false, Size(150, 122), 1.75, 6, 18, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish6_paths_stacks, 0.6};
    
    // - Fish 7 -
    
    // Right
    fish_path world2_fish7_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish7_wander_right_paths[] = {world2_fish7_path_wander_right};
    paths_stack world2_fish7_paths_stack_wander_right = {Location(), 0, 1, world2_fish7_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish7_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish7_wander_left_paths[] = {world2_fish7_path_wander_left};
    paths_stack world2_fish7_paths_stack_wander_left = {Location(), 0, 1, world2_fish7_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish7_paths_stacks[] = {world2_fish7_paths_stack_wander_right, world2_fish7_paths_stack_wander_left};
    fish_profile world2_fish7 = {fish7_image, &fish7_image_frames_amount, "fish 7", false, true, Size(250, 202), 1.75, 4, 10, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish7_paths_stacks, 0.8};
    
    // - Fish 8 -
    
    // Right
    fish_path world2_fish8_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish8_wander_right_paths[] = {world2_fish8_path_wander_right};
    paths_stack world2_fish8_paths_stack_wander_right = {Location(), 0, 1, world2_fish8_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish8_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish8_wander_left_paths[] = {world2_fish8_path_wander_left};
    paths_stack world2_fish8_paths_stack_wander_left = {Location(), 0, 1, world2_fish8_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish8_paths_stacks[] = {world2_fish8_paths_stack_wander_right, world2_fish8_paths_stack_wander_left};
    fish_profile world2_fish8 = {fish8_image, &fish8_image_frames_amount, "fish 8", true, true, Size(90, 81), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world2_fish8_paths_stacks, 0.7};
    
    // - Fish 9 -
    
    // Right
    fish_path world2_fish9_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish9_wander_right_paths[] = {world2_fish9_path_wander_right};
    paths_stack world2_fish9_paths_stack_wander_right = {Location(), 0, 1, world2_fish9_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish9_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish9_wander_left_paths[] = {world2_fish9_path_wander_left};
    paths_stack world2_fish9_paths_stack_wander_left = {Location(), 0, 1, world2_fish9_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish9_paths_stacks[] = {world2_fish9_paths_stack_wander_right, world2_fish9_paths_stack_wander_left};
    fish_profile world2_fish9 = {fish9_image, &fish9_image_frames_amount, "fish 9", false, true, Size(150, 123), 3, 12, 20, 0, 2, 30, 300, 1.2, 2, true, 2, world2_fish9_paths_stacks, 0.33};
    
    // - Fish 10 -
    
    // Right
    fish_path world2_fish10_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish10_wander_right_paths[] = {world2_fish10_path_wander_right};
    paths_stack world2_fish10_paths_stack_wander_right = {Location(), 0, 1, world2_fish10_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish10_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish10_wander_left_paths[] = {world2_fish10_path_wander_left};
    paths_stack world2_fish10_paths_stack_wander_left = {Location(), 0, 1, world2_fish10_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish10_paths_stacks[] = {world2_fish10_paths_stack_wander_right, world2_fish10_paths_stack_wander_left};
    fish_profile world2_fish10 = {fish10_image, &fish10_image_frames_amount, "fish 10", true, false, Size(480, 459), 2, 8, 25, 0, 2, 30, 300, 1.2, 2, false, 2, world2_fish10_paths_stacks, 0.3};
    
    // - Fish 11 -
    
    // Right
    fish_path world2_fish11_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_fish11_wander_right_paths[] = {world2_fish11_path_wander_right};
    paths_stack world2_fish11_paths_stack_wander_right = {Location(), 0, 1, world2_fish11_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world2_fish11_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_fish11_wander_left_paths[] = {world2_fish11_path_wander_left};
    paths_stack world2_fish11_paths_stack_wander_left = {Location(), 0, 1, world2_fish11_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world2_fish11_paths_stacks[] = {world2_fish11_paths_stack_wander_right, world2_fish11_paths_stack_wander_left};
    fish_profile world2_fish11 = {fish11_image, &fish11_image_frames_amount, "fish 11", false, true, Size(600, 510), 2, 1, 10, 0, 2, 30, 300, 1.2, 2, false, 2, world2_fish11_paths_stacks, 0.2};
    
    // - Crab 1 -
    
    // Right
    fish_path world2_crab1_path_wander_right = {10, 0, true, true, 100};
    fish_path world2_crab1_wander_right_paths[] = {world2_crab1_path_wander_right};
    paths_stack world2_crab1_paths_stack_wander_right = {Location(- X_COORD_OFFSET, world2.height - 50), world2.height - 50, 1, world2_crab1_wander_right_paths, false, false, true, false, true};
    
    // Left
    fish_path world2_crab1_path_wander_left = {10, 0, false, true, 100};
    fish_path world2_crab1_wander_left_paths[] = {world2_crab1_path_wander_left};
    paths_stack world2_crab1_paths_stack_wander_left = {Location(world2.width + X_COORD_OFFSET, world2.height - 50), world2.height - 50, 1, world2_crab1_wander_left_paths, false, false, true, false, false};
    
    // Fish profile.
    paths_stack world2_crab1_paths_stacks[] = {world2_crab1_paths_stack_wander_right, world2_crab1_paths_stack_wander_left};
    fish_profile world2_crab1 = {crab1_image, &crab1_image_frames_amount, "crab 1", true, true, Size(75, 75), 1, 1, 10, 0, 0, 30, 300, 10, 1, false, 2, world2_crab1_paths_stacks, 1};
    
    // -- Setup --
    
    fish_profile world2_fish_profiles_on_startup[] = {};
    fish_profile world2_available_fish[] = {world2_fish1, world2_fish2, world2_fish3, world2_fish4, world2_fish5, world2_fish6, world2_fish7, world2_fish8, world2_fish9, world2_fish10, world2_fish11, world2_crab1};
    FishNetwork world2_fish_network = FishNetwork(FPS, FISH_POPULATION, EAT_GROW_RATIO, &world2_grid, world2_fish_profiles_on_startup, 0, world2_available_fish, 12, X_COORD_OFFSET);
    world2_fish_network.update_boundaries(-X_COORD_OFFSET, world2.width + X_COORD_OFFSET, 0, world2.height, true);

    // ----- Final Set-ups World2 -----

    // Create and set-up the camera.
    Camera2D world2_camera = { 0 };
    world2_camera.target = (Vector2) { world2_my_fish.get_location().x, world2_my_fish.get_location().y };
    world2_camera.offset = (Vector2) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    world2_camera.rotation = 0;
    
    if (debug_camera) { world2_camera.zoom = 0.15; }
    else { world2_camera.zoom = 0.7; }
    
    // # ----- World 3 ----- #
    
    // Load Textures.
    Texture2D world3 = LoadTexture(PATH_WORLD3);
    
    // Create the grid.
    Grid world3_grid = Grid(GRID_COLS, GRID_ROWS, FISH_POPULATION, world3.width, world3.height);

    // ----- Create Entities -----
    
    // --- my fish ---
    
    Cell** world3_cells_within_my_fish = new Cell*[GRID_ROWS * GRID_COLS];
    MyFish world3_my_fish = MyFish(FPS, my_fish_image, &my_fish_image_frames_amount, false, Location(world3.width / 2, world3.height / 2), Size(150, 107), 15, 12, 0, 0, 0, 0, 1, 15, EAT_GROW_RATIO, 1.2, 10000, 0, true, FISH_POPULATION, world3_cells_within_my_fish, Location(100, SCREEN_HEIGHT - 75), Size(150, 20), 1, 2, FPS * 2, FPS * 5, Location(350, SCREEN_HEIGHT - 75), Size(150, 20), 1);

    // --- Fish Network ---

    // -- Fish Profiles --
    
    // - Fish 4 -
    
    // Right
    fish_path world3_fish4_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_fish4_wander_right_paths[] = {world3_fish4_path_wander_right};
    paths_stack world3_fish4_paths_stack_wander_right = {Location(), 0, 1, world3_fish4_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world3_fish4_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_fish4_wander_left_paths[] = {world3_fish4_path_wander_left};
    paths_stack world3_fish4_paths_stack_wander_left = {Location(), 0, 1, world3_fish4_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world3_fish4_paths_stacks[] = {world3_fish4_paths_stack_wander_right, world3_fish4_paths_stack_wander_left};
    fish_profile world3_fish4 = {fish4_image, &fish4_image_frames_amount, "fish 4", true, true, Size(90, 100), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world3_fish4_paths_stacks, 2};
    
    // - Fish 6 -
    
    // Right
    fish_path world3_fish6_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_fish6_wander_right_paths[] = {world3_fish6_path_wander_right};
    paths_stack world3_fish6_paths_stack_wander_right = {Location(), 0, 1, world3_fish6_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world3_fish6_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_fish6_wander_left_paths[] = {world3_fish6_path_wander_left};
    paths_stack world3_fish6_paths_stack_wander_left = {Location(), 0, 1, world3_fish6_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world3_fish6_paths_stacks[] = {world3_fish6_paths_stack_wander_right, world3_fish6_paths_stack_wander_left};
    fish_profile world3_fish6 = {fish6_image, &fish6_image_frames_amount, "fish 6", true, false, Size(150, 122), 2, 6, 18, 0, 2, 30, 300, 1.2, 2, true, 2, world3_fish6_paths_stacks, 0.6};
    
    // - Fish 8 -
    
    // Right
    fish_path world3_fish8_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_fish8_wander_right_paths[] = {world3_fish8_path_wander_right};
    paths_stack world3_fish8_paths_stack_wander_right = {Location(), 0, 1, world3_fish8_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world3_fish8_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_fish8_wander_left_paths[] = {world3_fish8_path_wander_left};
    paths_stack world3_fish8_paths_stack_wander_left = {Location(), 0, 1, world3_fish8_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world3_fish8_paths_stacks[] = {world3_fish8_paths_stack_wander_right, world3_fish8_paths_stack_wander_left};
    fish_profile world3_fish8 = {fish8_image, &fish8_image_frames_amount, "fish 8", true, true, Size(90, 81), 1.2, 15, 35, 0, 3, 30, 300, 1.2, 2, true, 2, world3_fish8_paths_stacks, 2};
    
    // - Fish 10 -
    
    // Right
    fish_path world3_fish10_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_fish10_wander_right_paths[] = {world3_fish10_path_wander_right};
    paths_stack world3_fish10_paths_stack_wander_right = {Location(), 0, 1, world3_fish10_wander_right_paths, false, false, false, false, true};
    
    // Left
    fish_path world3_fish10_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_fish10_wander_left_paths[] = {world3_fish10_path_wander_left};
    paths_stack world3_fish10_paths_stack_wander_left = {Location(), 0, 1, world3_fish10_wander_left_paths, false, false, false, false, false};
    
    // Fish profile.
    paths_stack world3_fish10_paths_stacks[] = {world3_fish10_paths_stack_wander_right, world3_fish10_paths_stack_wander_left};
    fish_profile world3_fish10 = {fish10_image, &fish10_image_frames_amount, "fish 10", true, false, Size(480, 459), 2, 8, 25, 0, 2, 30, 300, 1.2, 2, false, 2, world3_fish10_paths_stacks, 0.05};
    
    // - Crab 1 -
    
    // Right
    fish_path world3_crab1_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_crab1_wander_right_paths[] = {world3_crab1_path_wander_right};
    paths_stack world3_crab1_paths_stack_wander_right = {Location(- X_COORD_OFFSET, world3.height - 50), world3.height - 50, 1, world3_crab1_wander_right_paths, false, false, true, false, true};
    
    // Left
    fish_path world3_crab1_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_crab1_wander_left_paths[] = {world3_crab1_path_wander_left};
    paths_stack world3_crab1_paths_stack_wander_left = {Location(world3.width + X_COORD_OFFSET, world3.height - 50), world3.height - 50, 1, world3_crab1_wander_left_paths, false, false, true, false, false};
    
    // Fish profile.
    paths_stack world3_crab1_paths_stacks[] = {world3_crab1_paths_stack_wander_right, world3_crab1_paths_stack_wander_left};
    fish_profile world3_crab1 = {crab1_image, &crab1_image_frames_amount, "crab 1", true, true, Size(75, 75), 1, 1, 10, 0, 0, 30, 300, 10, 1, false, 2, world3_crab1_paths_stacks, 1};

    // - Crab 2 -
    
    // Right
    fish_path world3_crab2_path_wander_right = {10, 0, true, true, 100};
    fish_path world3_crab2_wander_right_paths[] = {world3_crab2_path_wander_right};
    paths_stack world3_crab2_paths_stack_wander_right = {Location(- X_COORD_OFFSET, world3.height - 50), world3.height - 50, 1, world3_crab2_wander_right_paths, false, false, true, false, true};
    
    // Left
    fish_path world3_crab2_path_wander_left = {10, 0, false, true, 100};
    fish_path world3_crab2_wander_left_paths[] = {world3_crab2_path_wander_left};
    paths_stack world3_crab2_paths_stack_wander_left = {Location(world3.width + X_COORD_OFFSET, world3.height - 50), world3.height - 50, 1, world3_crab2_wander_left_paths, false, false, true, false, false};
    
    // Fish profile.
    paths_stack world3_crab2_paths_stacks[] = {world3_crab2_paths_stack_wander_right, world3_crab2_paths_stack_wander_left};
    fish_profile world3_crab2 = {crab2_image, &crab2_image_frames_amount, "crab 2", false, true, Size(75, 75), 1, 1, 10, 0, 0, 30, 300, 10, 1, false, 2, world3_crab2_paths_stacks, 1};

    // - Jelly Fish 1 -
    
    // Falling down paths.
    fish_path world3_jeflly_fish1_falling_down1 = {4, 40, true, false, 40};
    fish_path world3_jeflly_fish1_falling_down2 = {5, 60, true, false, 40};
    fish_path world3_jeflly_fish1_falling_down3 = {20, 35, true, false, 40};
    fish_path world3_jeflly_fish1_moving_right = {13, 1, true, false, 50};
    
    fish_path world3_jelly_fish1_paths1[] = {world3_jeflly_fish1_falling_down1, world3_jeflly_fish1_moving_right};
    fish_path world3_jelly_fish1_paths2[] = {world3_jeflly_fish1_falling_down2, world3_jeflly_fish1_moving_right};
    fish_path world3_jelly_fish1_paths3[] = {world3_jeflly_fish1_falling_down3, world3_jeflly_fish1_moving_right};
    
    paths_stack world3_jelly_fish1_paths_stack_fall_down1 = {Location(), - jelly_fish1_image.height - 10, 2, world3_jelly_fish1_paths1, false, false, true, true, false};
    paths_stack world3_jelly_fish1_paths_stack_fall_down2 = {Location(), - jelly_fish1_image.height - 10, 2, world3_jelly_fish1_paths2, false, false, true, true, false};
    paths_stack world3_jelly_fish1_paths_stack_fall_down3 = {Location(), - jelly_fish1_image.height - 10, 2, world3_jelly_fish1_paths3, false, false, true, true, false};
    
    // Fish profile.
    paths_stack world3_jelly_fish1_paths_stacks[] = {world3_jelly_fish1_paths_stack_fall_down1, world3_jelly_fish1_paths_stack_fall_down2, world3_jelly_fish1_paths_stack_fall_down3};
    fish_profile world3_jeflly_fish1 = {jelly_fish1_image, &jeflly_fish1_image_frames_amount, "Jelly Fish", true, false, Size(200, 200), 1.3, 3, 20, 0, 7, 30, 300, 1.2, 2, true, 3, world3_jelly_fish1_paths_stacks, 1.5};
    
    // - Jelly Fish 2 -
    
    // Falling down paths.
    fish_path world3_jeflly_fish2_falling_down1 = {4, 40, true, false, 40};
    fish_path world3_jeflly_fish2_falling_down2 = {5, 60, true, false, 40};
    fish_path world3_jeflly_fish2_falling_down3 = {20, 35, true, false, 40};
    fish_path world3_jeflly_fish2_moving_right = {13, 1, true, false, 50};
    
    fish_path world3_jelly_fish2_paths1[] = {world3_jeflly_fish2_falling_down1, world3_jeflly_fish2_moving_right};
    fish_path world3_jelly_fish2_paths2[] = {world3_jeflly_fish2_falling_down2, world3_jeflly_fish2_moving_right};
    fish_path world3_jelly_fish2_paths3[] = {world3_jeflly_fish2_falling_down3, world3_jeflly_fish2_moving_right};
    
    paths_stack world3_jelly_fish2_paths_stack_fall_down1 = {Location(), - jelly_fish2_image.height - 10, 2, world3_jelly_fish2_paths1, false, false, true, true, false};
    paths_stack world3_jelly_fish2_paths_stack_fall_down2 = {Location(), - jelly_fish2_image.height - 10, 2, world3_jelly_fish2_paths2, false, false, true, true, false};
    paths_stack world3_jelly_fish2_paths_stack_fall_down3 = {Location(), - jelly_fish2_image.height - 10, 2, world3_jelly_fish2_paths3, false, false, true, true, false};
    
    // Fish profile.
    paths_stack world3_jelly_fish2_paths_stacks[] = {world3_jelly_fish2_paths_stack_fall_down1, world3_jelly_fish2_paths_stack_fall_down2, world3_jelly_fish2_paths_stack_fall_down3};
    fish_profile world3_jeflly_fish2 = {jelly_fish2_image, &jeflly_fish2_image_frames_amount, "Jelly Fish", true, false, Size(180, 180), 1.3, 3, 20, 0, 3, 30, 300, 1.2, 2, true, 3, world3_jelly_fish2_paths_stacks, 1.2};
    
    // -- Setup --
    
    fish_profile world3_fish_profiles_on_startup[] = {};
    fish_profile world3_available_fish[] = {world3_fish4, world3_fish6, world3_fish8, world3_fish10, world3_crab1, world3_crab2, world3_jeflly_fish1, world3_jeflly_fish2};
    FishNetwork world3_fish_network = FishNetwork(FPS, FISH_POPULATION, EAT_GROW_RATIO, &world3_grid, world3_fish_profiles_on_startup, 0, world3_available_fish, 8, X_COORD_OFFSET);
    world3_fish_network.update_boundaries(-X_COORD_OFFSET, world3.width + X_COORD_OFFSET, 0, world3.height, true);

    // ----- Final Set-ups World3 -----

    // Create and set-up the camera.
    Camera2D world3_camera = { 0 };
    world3_camera.target = (Vector2) { world3_my_fish.get_location().x, world3_my_fish.get_location().y };
    world3_camera.offset = (Vector2) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    world3_camera.rotation = 0;
    
    if (debug_camera) { world3_camera.zoom = 0.15; }
    else { world3_camera.zoom = 0.7; }
    
    // Play the main theme music.
    current_music = music_main_theme;
    PlaySound(current_music);
    
	// ----- Game Loop -----
	
	// As long as the Esc button or exit button were not pressed, continue to the next frame.
	while (!WindowShouldClose())
	{
        // --- Update Music ---
        
        // Play the current background music in loop.
        if (!IsSoundPlaying(current_music)) { PlaySound(current_music); }
        
        // --- Update Data ---
        
        if (current_screen == "Main Menu")
        {
            // Get the current position of the mouse.
            mouse_point = GetMousePosition();

            // The campain button was pressed.
            if (CheckCollisionPointRec(mouse_point, campain_button_frame) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                current_screen = "Map";
            }
        }
        
        else if (current_screen == "Map")
        {
            // Get the current position of the mouse.
            mouse_point = GetMousePosition();

            // World 1 was pressed.
            if (CheckCollisionPointRec(mouse_point, world1_button_frame) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Set the current world to world 1.
                current_screen = "World";
                current_world = 1;
                world = world1;
                my_fish = world1_my_fish;
                fish_network = world1_fish_network;
                grid = world1_grid;
                camera = world1_camera;
                grid.refresh_entity(&my_fish);
                fish_network.setup();
                
                // Play the music of world 1.
                StopSound(current_music);
                current_music = music_world1;
                PlaySound(current_music);
                
                continue;
            }
            
            // World 2 was pressed.
            if (game_save.world_checkpoint >= 2 && CheckCollisionPointRec(mouse_point, world2_button_frame) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Set the current world to world 2.
                current_screen = "World";
                current_world = 2;
                world = world2;
                my_fish = world2_my_fish;
                fish_network = world2_fish_network;
                grid = world2_grid;
                camera = world2_camera;
                grid.refresh_entity(&my_fish);
                fish_network.setup();
                
                // Play the music of world 2.
                StopSound(current_music);
                current_music = music_world2;
                PlaySound(current_music);
                
                continue;
            }
            
            // World 3 was pressed.
            if (game_save.world_checkpoint >= 3 && CheckCollisionPointRec(mouse_point, world3_button_frame) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Set the current world to world 3.
                current_screen = "World";
                current_world = 3;
                world = world3;
                my_fish = world3_my_fish;
                fish_network = world3_fish_network;
                grid = world3_grid;
                camera = world3_camera;
                grid.refresh_entity(&my_fish);
                fish_network.setup();
                
                // Play the music of world 3.
                StopSound(current_music);
                current_music = music_world3;
                PlaySound(current_music);
                
                continue;
            }
        }
        
        else if (is_victory || is_defeat)
        {
            // Keep playing the gifs in the background.
            my_fish.set_next_frame();
            fish_network.set_next_frame();
            
            // Get the current position of the mouse.
            mouse_point = GetMousePosition();

            // The back to map button was pressed.
            if (CheckCollisionPointRec(mouse_point, back_to_map_button_frame) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Reset the world.
                if (current_world == 1)
                {
                    // Reset my fish.
                    world1_my_fish.reset();
                    
                    // Reset the fish network.
                    world1_fish_network.reset();

                    // Reset the grid.
                    world1_grid.reset();
                    
                    // Reset the camera.
                    world1_camera.target = (Vector2) { world1_my_fish.get_location().x, world1_my_fish.get_location().y };
                    
                    // Stop the music.
                    StopSound(current_music);
                }
                
                // Reset the world.
                if (current_world == 2)
                {
                    // Reset my fish.
                    world2_my_fish.reset();
                    
                    // Reset the fish network.
                    world2_fish_network.reset();

                    // Reset the grid.
                    world2_grid.reset();
                    
                    // Reset the camera.
                    world2_camera.target = (Vector2) { world2_my_fish.get_location().x, world2_my_fish.get_location().y };
                    
                    // Stop the music.
                    StopSound(current_music);
                }
                
                // Reset the world.
                if (current_world == 3)
                {
                    // Reset my fish.
                    world3_my_fish.reset();
                    
                    // Reset the fish network.
                    world3_fish_network.reset();

                    // Reset the grid.
                    world3_grid.reset();
                    
                    // Reset the camera.
                    world3_camera.target = (Vector2) { world3_my_fish.get_location().x, world3_my_fish.get_location().y };
                    
                    // Stop the music.
                    StopSound(current_music);
                }
                
                // Change to the map, and reset the victory and defeat flags.
                current_screen = "Map";
                is_victory = false;
                is_defeat = false;
                
                // Change the music back to the theme.
                current_music = music_main_theme;
                PlaySound(music_main_theme);
            }
        }
        
        else if( current_screen == "World")
        {
            // ----- Handle world -----

            // - Boundaries Management -

            // Set the relevant boundaries for all the fish (its scaling considartions occurs within the fish update boundaries calls).
            // Needs to be updated each frame becuase the scaling of the fish can be changed.
            
            // Update my fish.
            my_fish_current_width = (int) ceil(sqrt(my_fish.get_scale()) * my_fish.get_size().width);
            my_fish_current_height = (int) ceil(sqrt(my_fish.get_scale()) * my_fish.get_size().height);
            my_fish.update_boundaries(ceil(my_fish_current_width / 2), world.width - ceil(my_fish_current_width / 2), ceil(my_fish_current_height / 2), world.height - ceil(my_fish_current_height / 2), false);
        
            // Update the fish network.
            fish_network.update_boundaries(-X_COORD_OFFSET, world.width + X_COORD_OFFSET, 0, world.height, true);
            
            // - Turbo.
            my_fish.update_turbo();
            
            // - Stunt.
            my_fish.update_stunt();
            
            // - User Input Management -

            // Handle the space bar.
            if (IsKeyPressed(KEY_SPACE)) { my_fish.apply_turbo(); }
            
            // Handle arrow keys strokes. They move the fish in the world.
            if (IsKeyDown(KEY_RIGHT)) { my_fish.move_right(); grid.refresh_entity(&my_fish); }
            if (IsKeyDown(KEY_LEFT)) { my_fish.move_left(); grid.refresh_entity(&my_fish); }
            if (IsKeyDown(KEY_UP)) { my_fish.move_up(); grid.refresh_entity(&my_fish); }
            if (IsKeyDown(KEY_DOWN)) { my_fish.move_down(); grid.refresh_entity(&my_fish); }
            
            // --- Entities Calculations ---
            
            // Remove all the eaten fish from the previous frame.
            fish_network.handle_eaten();
            
            // Release available fish.
            fish_network.load_available_fish(false);
            
            // Move all the fish in the fish network.
            fish_network.move();
            
            // --- Handle Collisions ---
            
            // A reference to the cells array.
            grid_cells = grid.get_cells();
            
            // The entities array of the current cell.
            GridEntity** entities_in_cell;
            
            // The rectangular frame of the two entities in the loop.
            Rectangle first_entity_rectangle;
            Rectangle second_entity_rectangle;    

            // Iterate over the cells of the grid.
            for (int row_index = 0; row_index < grid.get_rows_amount(); row_index++)
            {
                for (int col_index = 0; col_index < grid.get_columns_amount(); col_index++)
                {
                    // Save the amount of entities in the current cell.
                    current_cell_entities_amount = grid_cells[row_index][col_index] -> get_entities_counter();

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

            // --- Camera ---
            
            // Calculate the width and height of the camera screen (changed due to camera.zoom).
            camera_current_width = (1 / camera.zoom) * SCREEN_WIDTH;
            camera_current_height = (1 / camera.zoom) * SCREEN_HEIGHT;
            
            // Camera follows my fish movement.
            
            // If the fish is not close to the boundaries.
            if (my_fish.get_location().x - ceil(camera_current_width / 2) >= 0 && my_fish.get_location().x + ceil(camera_current_width / 2) < world.width) { camera_pos_x = my_fish.get_location().x; }
            if (my_fish.get_location().y - ceil(camera_current_height / 2) >= 0 && my_fish.get_location().y + ceil(camera_current_height / 2) < world.height) { camera_pos_y = my_fish.get_location().y; }

            // Update the camera position.
            if (!debug_camera) { camera.target = (Vector2){ camera_pos_x, camera_pos_y }; }

            // --- Prepare Gifs for drawing ---
            
            // Prepare all the fish to their next gif frame.
            my_fish.set_next_frame();        
            fish_network.set_next_frame();
            
            // Check if the game is over.
            if (!my_fish.is_alive())
            {
                is_defeat = true;
            }
            
            // Check if the fish has reached the required size.
            if (my_fish.is_victory())
            {
                // Update game status.
                is_victory = true;
                
                // Check if unlocked new world.
                if (game_save.world_checkpoint == current_world) { game_save.update_world_checkpoint(current_world + 1); }
            }
        }

        // ----- Draw -----
        
        BeginDrawing();
            
            if (current_screen == "Main Menu")
            {
                // Clear the background.
                ClearBackground(RAYWHITE);

                // Draw the main menu image.
                DrawTexture(main_menu, (int) floor (SCREEN_WIDTH / 2 - main_menu.width / 2), (int) floor(SCREEN_HEIGHT / 2 - main_menu.height / 2), WHITE);
                
                // Draw the campain button.
                DrawTexture(campain_button, campain_button_frame.x, campain_button_frame.y, WHITE);
            }
                    
            else if (current_screen == "Map")
            {
                // Clear the background.
                ClearBackground(RAYWHITE);

                // Draw the map image.
                DrawTexture(map, (int) floor (SCREEN_WIDTH / 2 - map.width / 2), (int) floor(SCREEN_HEIGHT / 2 - map.height / 2), WHITE);
                
                // Draw the worlds buttons.
                DrawTexture(world1_button, world1_button_frame.x, world1_button_frame.y, WHITE);
                if (game_save.world_checkpoint >= 2) { DrawTexture(world2_button, world2_button_frame.x, world2_button_frame.y, WHITE); }
                if (game_save.world_checkpoint >= 3) { DrawTexture(world3_button, world3_button_frame.x, world3_button_frame.y, WHITE); }
            }
            
            else if (current_screen == "World")
            {
                // Clear the background.
                ClearBackground(RAYWHITE);
                
                // Everything inside this scope, is being manipulated by the camera.
                // Every drawing outside this scope, will show up on the screen without being transformed by the camera.
                BeginMode2D(camera);
                    
                    // Draw the background.
                    DrawTexture(world, 0, 0, WHITE);
                    
                    // Debug, print the grid.
                    if (debug)
                    {
                        for (int row_index = 0; row_index < grid.get_rows_amount(); row_index++)
                        {
                            for (int col_index = 0; col_index < grid.get_columns_amount(); col_index++)
                            {
                                current_cell_entities_amount = grid_cells[row_index][col_index] -> get_entities_counter();
                                if (current_cell_entities_amount > 0) { DrawRectangle(col_index * grid.get_cell_width_pixels(), row_index * grid.get_cell_height_pixels(), grid.get_cell_width_pixels(), grid.get_cell_height_pixels(), GRAY); }
                                else { DrawRectangle(col_index * grid.get_cell_width_pixels(), row_index * grid.get_cell_height_pixels(), grid.get_cell_width_pixels(), grid.get_cell_height_pixels(), LIGHTGRAY); }
                            }
                        }
                    }
                    
                    // Debug, print the fish frames.
                    if (debug)
                    {
                        fish_network.print_frames();
                        Rectangle frame = my_fish.get_updated_rectangular_frame();
                        DrawRectangle(frame.x,frame.y, frame.width, frame.height, RED);
                    }
                    
                    // Draw the next gif frame of the fish.
                    fish_network.draw_next_frame();
                    my_fish.draw_next_frame();


                // The end of the drawings affected by the camera.
                EndMode2D();

                // Draw the current scale widget.
                my_fish.draw_scale_widget();
                
                // Draw the current turbo widget.
                my_fish.draw_turbo_widget();
            }
            
            // Draw on top of the world the is victory or is defeat windows.
            if (is_victory)
            {
                DrawTexture(victory, (int) floor(SCREEN_WIDTH / 2 - victory.width / 2), (int) floor(SCREEN_HEIGHT / 2 - victory.height / 2), WHITE);
                DrawTexture(back_to_map_button, back_to_map_button_frame.x, back_to_map_button_frame.y, WHITE);
            }
            if (is_defeat)
            {
                DrawTexture(defeat, (int) floor(SCREEN_WIDTH / 2 - defeat.width / 2), (int) floor(SCREEN_HEIGHT / 2 - defeat.height / 2), WHITE);
                DrawTexture(back_to_map_button, back_to_map_button_frame.x, back_to_map_button_frame.y, WHITE);
            }
            
        EndDrawing();
	}
	
	// ----- Close Game -----
	
    my_fish.delete_gif();
    fish_network.delete_network();
    
    // Remove the image.
    UnloadImage(my_fish_image);
    UnloadImage(fish1_image);
    
    // Close all the musics.
    //UnloadMusicStream(music_main_theme);
    UnloadSound(music_main_theme);
    UnloadSound(music_world1);
    UnloadSound(music_world2);
    
    CloseAudioDevice();
    
	// Close the game screen.
	CloseWindow();
    
    // Close the game progress file.
    game_save.quit();
}