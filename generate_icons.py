#!/usr/bin/env python3
"""
Generate PWA icons for ESP32 KNX Thermostat
Creates icon-192.png and icon-512.png with a thermostat design
"""

try:
    from PIL import Image, ImageDraw, ImageFont
    import os
except ImportError:
    print("PIL/Pillow is required. Install with: pip install Pillow")
    exit(1)

def create_thermostat_icon(size, output_path):
    """Create a thermostat icon with the specified size"""
    # Create a new image with transparent background
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Theme color: #1e88e5 (blue)
    theme_color = (30, 136, 229, 255)
    # Darker blue for borders
    border_color = (13, 71, 161, 255)
    # White for text/indicators
    white = (255, 255, 255, 255)
    # Light blue for background circle
    bg_color = (66, 165, 245, 255)
    
    # Calculate dimensions
    padding = size // 8
    center = size // 2
    radius = (size - padding * 2) // 2
    
    # Draw outer circle (background)
    draw.ellipse(
        [center - radius, center - radius, center + radius, center + radius],
        fill=bg_color,
        outline=border_color,
        width=max(2, size // 64)
    )
    
    # Draw inner circle (thermostat face)
    inner_radius = radius * 0.75
    draw.ellipse(
        [center - inner_radius, center - inner_radius, 
         center + inner_radius, center + inner_radius],
        fill=theme_color,
        outline=white,
        width=max(2, size // 96)
    )
    
    # Draw temperature indicator (arc/semicircle at top)
    # This represents the temperature scale
    arc_start = -180  # Start from top
    arc_end = 0  # End at top (180 degree arc)
    arc_bbox = [
        center - inner_radius * 0.9,
        center - inner_radius * 0.9,
        center + inner_radius * 0.9,
        center + inner_radius * 0.9
    ]
    draw.arc(arc_bbox, arc_start, arc_end, fill=white, width=max(3, size // 32))
    
    # Draw center dot (thermostat center)
    dot_radius = max(4, size // 32)
    draw.ellipse(
        [center - dot_radius, center - dot_radius,
         center + dot_radius, center + dot_radius],
        fill=white
    )
    
    # Draw temperature indicator line (pointing up-right)
    line_length = inner_radius * 0.5
    import math
    angle = math.radians(45)  # 45 degrees (up-right)
    end_x = center + line_length * math.cos(angle)
    end_y = center - line_length * math.sin(angle)
    draw.line(
        [center, center, end_x, end_y],
        fill=white,
        width=max(3, size // 32)
    )
    
    # Try to add text "T" for Thermostat (if size is large enough)
    if size >= 192:
        try:
            # Try to use a nice font, fallback to default if not available
            font_size = size // 4
            try:
                # Try to use a system font
                font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", font_size)
            except:
                try:
                    font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", font_size)
                except:
                    font = ImageFont.load_default()
            
            # Draw "T" in the center
            text = "T"
            bbox = draw.textbbox((0, 0), text, font=font)
            text_width = bbox[2] - bbox[0]
            text_height = bbox[3] - bbox[1]
            text_x = center - text_width // 2
            text_y = center - text_height // 2 - size // 16
            
            draw.text(
                (text_x, text_y),
                text,
                fill=white,
                font=font
            )
        except Exception as e:
            print(f"Could not add text: {e}")
    
    # Save the image
    img.save(output_path, 'PNG', optimize=True)
    print(f"Created {output_path} ({size}x{size})")

def create_favicon(size, output_path):
    """Create a simplified favicon"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    theme_color = (30, 136, 229, 255)
    white = (255, 255, 255, 255)
    
    center = size // 2
    radius = size // 2 - 2
    
    # Draw circle
    draw.ellipse(
        [center - radius, center - radius, center + radius, center + radius],
        fill=theme_color,
        outline=white,
        width=max(1, size // 16)
    )
    
    # Draw simple "T" or thermometer symbol
    if size >= 32:
        # Draw a simple thermometer-like shape
        line_width = max(2, size // 8)
        draw.line(
            [center, center - radius // 2, center, center + radius // 3],
            fill=white,
            width=line_width
        )
        # Draw bulb at bottom
        bulb_radius = size // 6
        draw.ellipse(
            [center - bulb_radius, center + radius // 3,
             center + bulb_radius, center + radius // 3 + bulb_radius * 2],
            fill=white
        )
    
    img.save(output_path, 'PNG', optimize=True)
    print(f"Created {output_path} ({size}x{size})")

def main():
    """Generate all required icons"""
    # Ensure data directory exists
    data_dir = 'data'
    if not os.path.exists(data_dir):
        os.makedirs(data_dir)
    
    # Generate PWA icons
    create_thermostat_icon(192, os.path.join(data_dir, 'icon-192.png'))
    create_thermostat_icon(512, os.path.join(data_dir, 'icon-512.png'))
    
    # Generate Apple touch icon (180x180)
    create_thermostat_icon(180, os.path.join(data_dir, 'apple-touch-icon.png'))
    
    # Generate favicon (32x32)
    create_favicon(32, os.path.join(data_dir, 'favicon-32x32.png'))
    
    print("\nIcons generated successfully!")
    print("Files created:")
    print("  - data/icon-192.png (PWA)")
    print("  - data/icon-512.png (PWA)")
    print("  - data/apple-touch-icon.png (iOS)")
    print("  - data/favicon-32x32.png (Browser)")

if __name__ == '__main__':
    main()

