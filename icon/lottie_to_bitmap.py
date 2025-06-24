from PIL import Image
import numpy as np
import pygame

def gif_to_cpp_matrix(gif_path, output_file, matrix_size=None):
    # Open the GIF file
    img = Image.open(gif_path)
    frames = []
    frame_index = 0
    height, width = matrix_size if matrix_size else img.size
    cpp_code = f"#include <vector>\n\nchar animation[{frame_index}][{height}][{width}] = {{\n"
    
    while True:
        # Convert to grayscale and binarize
        img_gray = img.convert('L')
        if matrix_size:
            img_gray = img_gray.resize(matrix_size, Image.LANCZOS)
        img_array = np.array(img_gray)
        # print(img_array)
        # img_gray.show()
        binary_matrix = (img_array < 64).astype(int)  # Convert to 0/1 based on threshold
        
        frames.append(binary_matrix)
        
        # Convert matrix to C++ format
        frame_str = f"    // Frame {frame_index}\n    {{\n" + ",\n".join(["        {" + ", ".join(map(str, row)) + "}" for row in binary_matrix]) + "\n    }"
        
        if frame_index > 0:
            cpp_code += ",\n"
        cpp_code += frame_str
        
        frame_index += 1
        
        # Move to the next frame
        try:
            img.seek(frame_index)
        except EOFError:
            break
    
    cpp_code += "\n};\n"
    
    # Write to output file
    with open(output_file, "w") as f:
        f.write(cpp_code)
    
    print(f"C++ matrix saved to {output_file}")
    
    # Play animation using pygame
    # pygame.init()
    # screen = pygame.display.set_mode((width * 10, height * 10))  # Scale up for visibility
    # clock = pygame.time.Clock()
    # running = True
    
    # while running:
    #     for event in pygame.event.get():
    #         if event.type == pygame.QUIT:
    #             running = False
        
    #     for frame in frames:
    #         screen.fill((0, 0, 0))  # Clear screen
    #         for y in range(height):
    #             for x in range(width):
    #                 if frame[y, x] == 1:
    #                     pygame.draw.rect(screen, (255, 255, 255), (x * 10, y * 10, 10, 10))
    #         pygame.display.flip()
    #         clock.tick(10)  # Control frame rate
    
    # pygame.quit()

# Example usage
gif_to_cpp_matrix(
    "system-solid-114-edit-pencil-rename-hover-edit.gif",
    "output.cpp", matrix_size=(24, 24))
