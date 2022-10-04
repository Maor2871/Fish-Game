#include <raylib.h>
#include <string>
#include <iostream>
using namespace std;


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


class MyGif
{
    /*
        Load a gif to the screen.
        
        Notes:
            - The gif is loaded as a texture, and manipulated as a texture.
    */
    
    // Define the protected attributes.
    protected:
    
        // The path to the gif.
        const char* file_path;
        
        // The location of the gif on the screen. Reffering to center of the gif.
        Location location;
        
        // The size of the gif on the screen.
        Size size;
        
        // The scale of the gif.
        float scale;
        
        // The rotation of the gif.
        float rotation;
        
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
        MyGif(const char* new_file_path, Location new_location, Size new_size, float new_scale = 1, float new_rotation = 0)
        {
            // Save the path to the gif file.
            file_path = new_file_path;

            // Save the location and size of the image.
            location.set_location(new_location);
            size.set_size(new_size);
            
            // Save the scale of the gif.
            scale = new_scale;
            
            // Save the rotation of the gif.
            rotation = 0;
            
            // Do not flip the texture on initialization.
            is_flip_horizontal = false;
            is_flip_vertical = false;

            // Create the image instance.
            my_gif_image = LoadImageAnim(file_path, &frames_amount);
            
            // Create the texture instance.
            my_gif_texture = LoadTextureFromImage(my_gif_image);        
        }
        
        void flip_horizontal() {is_flip_horizontal = true;}
        
        void flip_vertical() {is_flip_vertical = true;}
        
        void unflip_horizontal() {is_flip_horizontal = false;}
        
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
    
    private:
    
        // The speed of the fish on the x and y axes, pixels/frame;
        float speed_x, speed_y;
        
        // The location boundaries of the fish.
        int left_boundary, right_boundary, top_boundary, bottom_boundary;
       
    public:
        
        // Counstructor.
        Fish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary= 0, int new_right_boundary = 0, int new_top_boundary = 0, int new_bottom_boundary = 0, float new_scale = 1, float new_rotation = 0) : MyGif(file_path, new_location, new_size, new_scale, new_rotation)
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
    
        MyFish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, int new_left_boundary = 0, int new_right_boundary = 0, int new_top_boundary = 0, int new_bottom_boundary = 0, float new_scale = 1, float new_rotation = 0) : Fish(file_path, new_location, new_size, new_speed_x, new_speed_y, new_left_boundary, new_right_boundary, new_top_boundary, new_bottom_boundary, new_scale, new_rotation)
        {
        }   
};


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
    const char* path_my_fish = "Textures/my_fish.gif";
    const char* path_world1 = "Textures/Worlds/world1.png";
   
	// --- GUI Initialization ---
	
	// Screen set-up.
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
	
	// Fps declaration.
	SetTargetFPS(FPS);
    
    // Load Textures.
    Texture2D world1 = LoadTexture(path_world1);

    // Load Gifs.
    MyFish my_fish = MyFish(path_my_fish, Location(world1.width / 2, world1.height / 2), Size(300, 300), 15, 12);
	
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
        
        // Set the relevant boundaries for my fish (its scaling considartion occurs within the fish update boundaries).
        my_fish.update_boundaries((SCREEN_WIDTH / camera.zoom) / 2, world1.width - (SCREEN_WIDTH / camera.zoom) / 2, (SCREEN_HEIGHT / camera.zoom) / 2, world1.height - (SCREEN_HEIGHT / camera.zoom) / 2);
        
        // Handle arrow keys strokes. They move the fish in the world.
        if (IsKeyDown(KEY_RIGHT)) {my_fish.move_right();}
        if (IsKeyDown(KEY_LEFT)) {my_fish.move_left();}
        if (IsKeyDown(KEY_UP)) {my_fish.move_up();}
        if (IsKeyDown(KEY_DOWN)) {my_fish.move_down();}
        
        // Camera follows my fish movement.
        camera.target = (Vector2){ my_fish.get_location().x, my_fish.get_location().y };
        
        // Prepare my fish for the next gif frame.
		my_fish.set_next_frame();
        
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
            
            // The end of the drawings affected by the camera.
            EndMode2D();

        EndDrawing();
	}
	
	// ----- Close Game -----
	
    my_fish.delte_gif();
    
	// Close the game screen.
	CloseWindow();
}