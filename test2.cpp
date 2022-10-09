#include "raylib.h"
#include <iostream>
#include <cmath>
using namespace std;



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
        int get_scale() { return scale; }
        int get_rotation() { return rotation; }
};

// A description of a path.
struct fish_path
{
    int speed_x;
    int speed_y;
    int is_moving_right;
    int is_moving_top;
    int current_frames_left;
};


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

    public:

        // The array of cells, as pointers, that the entity is currently within.
        // cell_within is public due to the "referencing each other" conflic of Cell and GridEntity. The cells that the entity is within, directly manipulates cells_witihn from outside this class.
        //Cell** cells_within;

    public:

        // Constructor.
        GridEntity(Location new_location, Size new_size, int new_scale, int new_rotation, int new_max_cells_within) : Entity(new_location, new_size, new_scale, new_rotation)
        {
            // Save the maximum amount of cells might be within.
            max_cells_within = new_max_cells_within; 
            
            // Create the cells array. Its length is exactly max_cells_within.
            //cells_within = new_cells_within;
            
            // Not on the grid yet on initialization.
            current_amount_of_cells_within = 0;           
        }
        
        // Defalut constructor.
        GridEntity() : Entity()
        {
            max_cells_within = 0;
            //cells_within = NULL;
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
        //Cell** get_cells_within() { return cells_within; }
        
        // The function returns the amount of cells curretnly within.
        int get_amount_of_cells_within() { return current_amount_of_cells_within; }
        
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
            //new_entity -> cells_within[new_entity -> get_amount_of_cells_within()] = this;
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
        MyGif(const char* new_file_path, Location new_location, Size new_size, int new_scale, int new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within) : GridEntity(new_location, new_size, new_scale, new_rotation, new_max_cells_within)
        {
            cout << "starting MyGif const.\n";
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
            cout << "finishing my gif const.\n";
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
            Rectangle destination = {location.x, location.y, scale * size.width, scale * size.width};
            
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
        Fish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within) : MyGif(file_path, new_location, new_size, new_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within)
        {
            cout << "starting Fish const.\n";
            // Set the speed of the fish.
            speed_x = new_speed_x;
            speed_y = new_speed_y;
            
            // Set the boundaries of the fish.
            left_boundary = new_left_boundary;
            right_boundary = new_right_boundary;
            top_boundary = new_top_boundary;
            bottom_boundary = new_bottom_boundary;
        }
        
        //void set_network(FishNetwork* new_network) { network = new_network; }
        
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
        void boundary_exceed() 
        {
            // Tell the network of the fish, if exists, that the fish is gone.
            //if (network != NULL)
            //    network -> delete_fish(this);
            
            delete_fish();
        }
        
        // The function deletes the fish from the world.
        // If the gif is not a part of a network, it is required to manualy remove it from the grid.
        void delete_fish()
        {
            delete_gif();
        }
        
        // The function handles a collision between the fish and another GridEntity.
        void handle_collision(GridEntity* colided_with_entity)
        {
            cout << "\n\nHorray fish collided!\n\n";
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
           
        // The range of speeds in both axes.
        int min_speed_x;
        int max_speed_x;
        int min_speed_y;
        int max_speed_y;
   
    public:
        
        // Constructor.
        // new_paths_count_in_paths_stack should state the number of stacks which are saved in the received paths_stack.
        // The location is relevant if there is no paths_stack or is_initial_location is false;
        WanderFish(const char* file_path, Location new_location, bool is_initial_left_location, Size new_size, float new_min_speed_x, float new_max_speed_x, float new_min_speed_y, float new_max_speed_y, int new_min_path_frames, int new_max_path_frames, int new_left_boundary, int new_right_boundary, int new_top_boundary, int new_bottom_boundary, float new_scale, float new_rotation, bool new_is_facing_left_on_startup, int new_max_cells_within) : Fish(file_path, new_location, new_size, 0, 0, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_rotation, new_is_facing_left_on_startup, new_max_cells_within)
        {
            cout << "in wander fish const.\n";
            // Set the range of speeds on both axes.
            min_speed_x = new_min_speed_x;
            max_speed_x = new_max_speed_x;
            min_speed_y = new_min_speed_y;
            max_speed_y = new_max_speed_y;
            
            // Set the range of a new path frames.
            min_path_frames = new_min_path_frames;
            max_path_frames = new_max_path_frames;
            
            // Set the paths stack, and save the number of elements in it.
            paths_stack_index = 0;
            /*
            // If there are paths in stack path, pull out the first one to be the current path.
            if (my_paths_stack.length > 0) 
            {
                // If initial location is relevant.
                if (my_paths_stack.is_initial_location) { location.set_location(my_paths_stack.initial_location); }
                
                // Otherwise, randomize it.
                else { set_random_initial_location(is_initial_left_location); }
                
                // Set the first path as the current path.
                current_path = my_paths_stack.paths[0];
            }
            
            // Otherwise, generate new path and assign it.
            else 
            {
                current_path = generate_new_path();
                
                set_random_initial_location(is_initial_left_location);
            }
            */
            current_path = generate_new_path();
                
            //set_random_initial_location(is_initial_left_location);
            // Update the properties of the wander fish to match the properties of the current path.
            //match_path_in_fish();
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




//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    const char* PATH_WORLD1 = "Textures/Worlds/world1.png";
    const char* PATH_FISH1 = "Textures/Fish/fish1.gif";
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Load Textures.
    Texture2D world1 = LoadTexture(PATH_WORLD1);
    
    WanderFish test = WanderFish(PATH_FISH1, Location(), false, Size(150, 150), 10, 15, 0, 1, 100, 100, 0, world1.width, 0, world1.height, 1, 0, true, 15);
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}