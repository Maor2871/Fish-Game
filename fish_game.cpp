#include <raylib.h>
#include <string>
#include <iostream>
using namespace std;


class Location
{
    private:
    
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
        
        int get_x() { return x; }
        
        int get_y() { return y; }
        
        void set_location(Location location)
        {
            x = location.get_x();
            y = location.get_y();
        }
        
        void set_x(int x) { x = x; }
        
        void set_y(int y) { y = y; }
};


class Size
{
    private:
        
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
        
        int get_width() { return width; }
        
        int get_height() { return height; }
        
        void set_size(Size size)
        {
            width = size.width;
            height = size.height;
        }
        
        void set_width(int width) { width = width; }
        
        void set_height(int height) { height = height; }
};


class MyGif
{
    /*
        Load a gif to the screen.
        
        Notes:
            - The gif is loaded as a texture, and manipulated as a texture.
    */
    
    // Define the private attributes.
    private:
    
        // The path to the gif.
        const char* file_path;
        
        // The location of the gif on the screen. Reffering to top left corner.
        Location location;
        
        // The size of the gif on the screen.
        Size size;
        
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
        MyGif(const char* file_path, Location new_location, Size new_size)
        {
            // Save the path to the gif file.
            file_path = file_path;

            // Save the location and size of the image.
            location.set_location(new_location);
            size.set_size(new_size);

            // Create the image instance.
            my_gif_image = LoadImageAnim(file_path, &frames_amount);
            
            // Create the texture instance.
            my_gif_texture = LoadTextureFromImage(my_gif_image);        
        }
        
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
            DrawTextureEx(my_gif_texture, (Vector2) {location.get_x(), location.get_y()}, 0, (float)size.get_width() / my_gif_texture.width, WHITE);
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
    MyGif my_fish = MyGif(path_my_fish, Location(100, 100), Size(300, 300));
	
	// ----- Game Loop -----
	
	// As long as the Esc button or exit button were not pressed, continue to the next frame.
	while (!WindowShouldClose())
	{
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