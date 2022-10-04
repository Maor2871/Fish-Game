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
            y = location.x;
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
    
    // Define the private attributes.
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
       
    public:
    
        Fish(const char* file_path, Location new_location, Size new_size, float new_speed_x, float new_speed_y, float new_scale = 1, float new_rotation = 0) : MyGif(file_path, new_location, new_size, new_scale, new_rotation)
        {
            // Set the speed of the fish.
            speed_x = new_speed_x;
            speed_y = new_speed_y;
        }
        
        void move_left() {location.x -= speed_x; flip_horizontal();}
        void move_right() {location.x += speed_x; unflip_horizontal();}
        void move_up() {location.y -= speed_y;}
        void move_down() {location.y += speed_y;}
};


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
    const char* path_background1 = "Textures/Backgrounds/background1.png";
    
	// --- GUI Initialization ---
	
	// Screen set-up.
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
	
	// Fps declaration.
	SetTargetFPS(FPS);
    
    // Load Textures.
    Texture2D background1 = LoadTexture(path_background1);
    
    // Load Gifs.
    Fish my_fish = Fish(path_my_fish, Location(150, 150), Size(300, 300), 8, 3);
	
	// ----- Game Loop -----
	
	// As long as the Esc button or exit button were not pressed, continue to the next frame.
	while (!WindowShouldClose())
	{
        
        // Handle arrow keys strokes.
        if (IsKeyDown(KEY_RIGHT)) {my_fish.move_right();}
        if (IsKeyDown(KEY_LEFT)) {my_fish.move_left();}
        if (IsKeyDown(KEY_UP)) {my_fish.move_up();}
        if (IsKeyDown(KEY_DOWN)) {my_fish.move_down();}
        
		my_fish.set_next_frame();

        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawTexture(background1, 0, 0, WHITE);
            
            my_fish.draw_next_frame();

        EndDrawing();
	}
	
	// ----- Close Game -----
	
    my_fish.delte_gif();
    
	// Close the game screen.
	CloseWindow();
}