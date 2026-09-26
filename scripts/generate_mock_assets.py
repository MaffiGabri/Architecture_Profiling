#!/usr/bin/env python3
"""
scripts/generate_mock_assets.py

Architecture Profiling Mock Asset & Metadata Generator
Generates 10 distinct, procedurally rendered 800x600 PNG architectural images
in shared/images/ and writes the authoritative metadata contract to
shared/data/styles.json and shared/data/architecture_images.json.

Complies strictly with Pillow 10.3.0 rendering discipline:
- Separate RGBA layers for alpha compositing (preventing alpha cutout defects)
- ImageDraw.textbbox for all dynamic text bounds (no deprecated textsize)
- Robust multi-tier Windows font loading cascade
- Multi-stop vertical and spotlight background gradients
- Architectural blueprint drafting grids with registration crosshairs
- Translucent acrylic bottom pill badge with category accents
- 10 distinct tectonic architectural silhouettes
"""

import json
import math
import os
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, List, Tuple

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("ERROR: Pillow is required to generate mock assets. Run: pip install Pillow")
    sys.exit(1)


# ==============================================================================
# 1. Multi-Tier Safe Font Loader
# ==============================================================================

def get_font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    """
    Multi-tier font loader:
    Tier 1: Windows Segoe UI (Clean, native Windows modern typography)
    Tier 2: Windows Arial (Universally installed fallback)
    Tier 3: Linux / Container fonts (DejaVu Sans / Liberation Sans)
    Tier 4: Pillow 10.3.0 dynamic FreeType default loader
    """
    candidates = []
    if bold:
        candidates = [
            "segoeuib.ttf",
            "C:/Windows/Fonts/segoeuib.ttf",
            "arialbd.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        ]
    else:
        candidates = [
            "segoeui.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        ]

    for path in candidates:
        try:
            return ImageFont.truetype(path, size=size)
        except (OSError, IOError):
            continue

    try:
        return ImageFont.load_default(size=size)
    except TypeError:
        return ImageFont.load_default()


# ==============================================================================
# 2. Gradient & Blueprint Grid Generators
# ==============================================================================

def create_vertical_gradient(
    width: int,
    height: int,
    stops: List[Tuple[float, Tuple[int, int, int]]]
) -> Image.Image:
    """
    Creates a smooth vertical multi-stop gradient using bilinear resampling.
    stops: list of (normalized_pos, (R, G, B)) sorted by position.
    """
    strip = Image.new("RGBA", (1, 256))
    for y in range(256):
        pos = y / 255.0
        # Find enclosing stops
        c1, c2 = stops[0][1], stops[-1][1]
        p1, p2 = 0.0, 1.0
        for i in range(len(stops) - 1):
            if stops[i][0] <= pos <= stops[i + 1][0]:
                p1, c1 = stops[i]
                p2, c2 = stops[i + 1]
                break
        t = 0.0 if p2 == p1 else (pos - p1) / (p2 - p1)
        r = int(c1[0] + (c2[0] - c1[0]) * t)
        g = int(c1[1] + (c2[1] - c1[1]) * t)
        b = int(c1[2] + (c2[2] - c1[2]) * t)
        strip.putpixel((0, y), (r, g, b, 255))
    return strip.resize((width, height), resample=Image.Resampling.BILINEAR)


def create_radial_spotlight(
    width: int,
    height: int,
    cx: int,
    cy: int,
    radius: int,
    color: Tuple[int, int, int],
    max_alpha: int = 120
) -> Image.Image:
    """Creates a radial glow spotlight overlay on an independent transparent layer."""
    layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    steps = 40
    for i in range(steps, 0, -1):
        r = int(radius * (i / steps))
        alpha = int(max_alpha * (1.0 - (i / steps)) ** 1.5)
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(color[0], color[1], color[2], alpha))
    return layer


def draw_blueprint_grid(width: int, height: int, is_light: bool = False) -> Image.Image:
    """
    Draws an architectural blueprint drafting grid onto an independent RGBA layer.
    Includes minor grid (25px), major grid (100px), crosshairs (+), and 20px frame.
    """
    grid_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(grid_layer)

    if is_light:
        minor_col = (30, 41, 59, 14)
        major_col = (30, 41, 59, 36)
        cross_col = (15, 23, 42, 60)
        border_col = (30, 41, 59, 45)
    else:
        minor_col = (255, 255, 255, 14)
        major_col = (255, 255, 255, 34)
        cross_col = (255, 255, 255, 75)
        border_col = (255, 255, 255, 45)

    # 1. Minor grid (every 25 px)
    for x in range(25, width, 25):
        draw.line([(x, 20), (x, height - 20)], fill=minor_col, width=1)
    for y in range(25, height, 25):
        draw.line([(20, y), (width - 20, y)], fill=minor_col, width=1)

    # 2. Major grid (every 100 px)
    for x in range(100, width, 100):
        draw.line([(x, 20), (x, height - 20)], fill=major_col, width=1)
    for y in range(100, height, 100):
        draw.line([(20, y), (width - 20, y)], fill=major_col, width=1)

    # 3. Registration crosshairs at major grid intersections
    for x in range(100, width, 100):
        for y in range(100, height, 100):
            draw.line([(x - 5, y), (x + 5, y)], fill=cross_col, width=1)
            draw.line([(x, y - 5), (x, y + 5)], fill=cross_col, width=1)

    # 4. Framing border at 20 px inset with tick marks
    draw.rectangle([20, 20, width - 20, height - 20], outline=border_col, width=1)
    for x in range(50, width - 20, 50):
        draw.line([(x, 17), (x, 23)], fill=cross_col, width=1)
        draw.line([(x, height - 23), (x, height - 17)], fill=cross_col, width=1)
    for y in range(50, height - 20, 50):
        draw.line([(17, y), (23, y)], fill=cross_col, width=1)
        draw.line([(width - 23, y), (width - 17, y)], fill=cross_col, width=1)

    return grid_layer


# ==============================================================================
# 3. Bottom Translucent Pill Badge Overlay
# ==============================================================================

def hex_to_rgb(hex_str: str) -> Tuple[int, int, int]:
    hex_clean = hex_str.lstrip("#")
    return (
        int(hex_clean[0:2], 16),
        int(hex_clean[2:4], 16),
        int(hex_clean[4:6], 16)
    )


def draw_bottom_pill_badge(
    base: Image.Image,
    style_id: int,
    style_en: str,
    style_it: str,
    category_name_en: str,
    category_color_hex: str,
    tagline: str
) -> Image.Image:
    """
    Renders the modern translucent glassmorphic pill badge anchored at the bottom
    (centered X=[80, 720], Y=[518, 574], r=28 px).
    Composited onto base via an independent RGBA layer.
    """
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    pill_x1, pill_y1 = 80, 518
    pill_x2, pill_y2 = 720, 574
    pill_radius = 28

    # 1. Glassmorphic acrylic background and rim
    draw.rounded_rectangle(
        [pill_x1, pill_y1, pill_x2, pill_y2],
        radius=pill_radius,
        fill=(15, 23, 42, 230),  # Smoked dark acrylic, 90% opacity
        outline=(255, 255, 255, 65),
        width=1
    )

    # 2. Circular ID Badge
    badge_cx, badge_cy = pill_x1 + 34, 546
    badge_r = 17
    cat_rgb = hex_to_rgb(category_color_hex)
    draw.ellipse(
        [badge_cx - badge_r, badge_cy - badge_r, badge_cx + badge_r, badge_cy + badge_r],
        fill=(cat_rgb[0], cat_rgb[1], cat_rgb[2], 255),
        outline=(255, 255, 255, 180),
        width=1
    )

    font_badge = get_font(13, bold=True)
    id_str = f"#{style_id:02d}"
    bbox_id = draw.textbbox((0, 0), id_str, font=font_badge)
    id_w = bbox_id[2] - bbox_id[0]
    id_h = bbox_id[3] - bbox_id[1]

    # Adaptive text fill based on relative background luminance (WCAG AA/AAA)
    cat_lum = 0.2126 * (cat_rgb[0] / 255.0) ** 2.2 + 0.7152 * (cat_rgb[1] / 255.0) ** 2.2 + 0.0722 * (cat_rgb[2] / 255.0) ** 2.2
    badge_text_fill = (15, 23, 42, 255) if cat_lum > 0.30 else (255, 255, 255, 255)
    draw.text(
        (badge_cx - id_w / 2, badge_cy - id_h / 2 - 1),
        id_str,
        fill=badge_text_fill,
        font=font_badge
    )

    # 3. Right-aligned Category Capsule Tag
    font_cat = get_font(9, bold=True)
    cat_text = category_name_en.upper()
    bbox_cat = draw.textbbox((0, 0), cat_text, font=font_cat)
    cat_w = bbox_cat[2] - bbox_cat[0]
    cat_h = bbox_cat[3] - bbox_cat[1]

    capsule_pad_h = 10
    capsule_w = cat_w + capsule_pad_h * 2
    capsule_h = 22
    capsule_x2 = pill_x2 - 16
    capsule_x1 = capsule_x2 - capsule_w
    capsule_y1 = pill_y1 + (pill_y2 - pill_y1 - capsule_h) // 2
    capsule_y2 = capsule_y1 + capsule_h

    # Capsule background
    draw.rounded_rectangle(
        [capsule_x1, capsule_y1, capsule_x2, capsule_y2],
        radius=11,
        fill=(255, 255, 255, 28),
        outline=(255, 255, 255, 55),
        width=1
    )
    # Capsule text
    draw.text(
        (capsule_x1 + capsule_pad_h, capsule_y1 + (capsule_h - cat_h) / 2 - 1),
        cat_text,
        fill=(203, 213, 225, 255),
        font=font_cat
    )

    # 4. Bilingual Title & Sub-caption
    text_x = pill_x1 + 64
    font_title = get_font(15, bold=True)

    title_y = 527
    draw.text((text_x, title_y), style_en, fill=(248, 250, 252, 255), font=font_title)

    sub_caption = f"{style_it}  •  {tagline}"
    sub_y = 548
    sub_font_size = 11
    font_sub = get_font(sub_font_size, bold=False)
    # Dynamic safeguard reducing font size if clearance drops below 30px
    while capsule_x1 - draw.textbbox((text_x, sub_y), sub_caption, font=font_sub)[2] < 30 and sub_font_size > 8:
        sub_font_size -= 1
        font_sub = get_font(sub_font_size, bold=False)

    draw.text((text_x, sub_y), sub_caption, fill=(148, 163, 184, 255), font=font_sub)

    return Image.alpha_composite(base, layer)


# ==============================================================================
# 4. Procedural Vector Drawing Routines for the 10 Architectural Styles
# ==============================================================================

def draw_style_01_classical(base: Image.Image) -> Image.Image:
    """Style 01: Classical Antiquity (Parthenon Peristyle)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    # Crepidoma 3 stepped plinth platforms
    draw.rectangle([90, 495, 710, 515], fill=(194, 182, 157, 255), outline=(130, 120, 100, 255), width=1)
    draw.rectangle([115, 475, 685, 495], fill=(221, 210, 189, 255), outline=(150, 140, 120, 255), width=1)
    draw.rectangle([140, 455, 660, 475], fill=(239, 232, 216, 255), outline=(180, 170, 150, 255), width=1)

    # Octastyle (8 Doric Columns)
    col_x_coords = [165 + i * (470 / 7) for i in range(8)]
    for xc in col_x_coords:
        # Shaft with entasis taper (base width 28, necking 22)
        base_w, neck_w = 14, 11
        shaft_pts = [
            (xc - neck_w, 250),
            (xc + neck_w, 250),
            (xc + base_w, 455),
            (xc - base_w, 455)
        ]
        draw.polygon(shaft_pts, fill=(245, 240, 230, 255), outline=(140, 130, 115, 255))
        # Left highlight, right shadow
        draw.polygon([(xc - neck_w, 250), (xc, 250), (xc, 455), (xc - base_w, 455)], fill=(255, 252, 245, 120))
        draw.polygon([(xc, 250), (xc + neck_w, 250), (xc + base_w, 455), (xc, 455)], fill=(168, 155, 132, 130))
        # 3 Fluting vertical shadow grooves
        for fx in [-6, 0, 6]:
            draw.line([(xc + fx * 0.8, 252), (xc + fx, 453)], fill=(150, 140, 120, 100), width=1)

        # Capital: Echinus & Abacus
        # Echinus (flaring trapezoid)
        draw.polygon([(xc - 15, 238), (xc + 15, 238), (xc + 11, 250), (xc - 11, 250)], fill=(235, 227, 212, 255), outline=(140, 130, 115, 255))
        # Abacus (square slab)
        draw.rectangle([xc - 17, 228, xc + 17, 238], fill=(245, 238, 225, 255), outline=(150, 140, 125, 255), width=1)

    # Entablature
    # Epistyle / Architrave
    draw.rectangle([135, 202, 665, 228], fill=(230, 222, 208, 255), outline=(140, 130, 115, 255), width=1)
    # Frieze
    draw.rectangle([135, 165, 665, 202], fill=(215, 205, 190, 255), outline=(130, 120, 105, 255), width=1)
    # Triglyphs aligned above each column and intermediate bays
    triglyph_xs = [xc for xc in col_x_coords] + [(col_x_coords[i] + col_x_coords[i + 1]) / 2 for i in range(7)]
    for tx in triglyph_xs:
        draw.rectangle([tx - 6, 167, tx + 6, 200], fill=(185, 172, 155, 255), outline=(120, 110, 95, 255), width=1)
        for gx in [-3, 0, 3]:
            draw.line([(tx + gx, 169), (tx + gx, 198)], fill=(120, 110, 95, 255), width=1)

    # Projecting Cornice (Geison)
    draw.rectangle([115, 148, 685, 165], fill=(245, 238, 225, 255), outline=(150, 140, 125, 255), width=1)

    # Pediment & Tympanum
    # Recessed Tympanum with relief shadow
    draw.polygon([(145, 148), (400, 80), (655, 148)], fill=(148, 135, 115, 255), outline=(110, 100, 85, 255))
    # Tympanum sculptural relief figures
    for sx in range(200, 600, 28):
        dist = abs(sx - 400)
        h = max(10, int(52 - dist * 0.18))
        draw.ellipse([sx - 6, 144 - h, sx + 6, 146], fill=(225, 218, 202, 220))

    # Outer Raking Cornice
    draw.line([(115, 148), (400, 70)], fill=(248, 242, 230, 255), width=6)
    draw.line([(685, 148), (400, 70)], fill=(248, 242, 230, 255), width=6)
    draw.polygon([(115, 148), (400, 70), (400, 78), (145, 148)], fill=(255, 250, 240, 255))
    draw.polygon([(685, 148), (400, 70), (400, 78), (655, 148)], fill=(220, 210, 195, 255))

    # Acroterion finials (apex and corners)
    draw.ellipse([394, 52, 406, 68], fill=(217, 119, 6, 255), outline=(255, 230, 120, 255))
    draw.polygon([(390, 70), (400, 52), (410, 70)], fill=(217, 119, 6, 255))
    draw.ellipse([110, 138, 122, 150], fill=(217, 119, 6, 255))
    draw.ellipse([678, 138, 690, 150], fill=(217, 119, 6, 255))

    return Image.alpha_composite(base, layer)


def draw_style_02_gothic(base: Image.Image) -> Image.Image:
    """Style 02: Gothic (Notre-Dame Westwork Cathedral)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    stone_dark = (35, 42, 60, 255)
    stone_mid = (55, 65, 88, 255)
    stone_light = (85, 98, 125, 255)

    # Lateral Flying Buttress piers & flyers
    draw.rectangle([75, 220, 120, 510], fill=stone_dark, outline=stone_mid, width=1)
    draw.rectangle([680, 220, 725, 510], fill=stone_dark, outline=stone_mid, width=1)
    # Flying arches (flyers)
    draw.arc([100, 220, 260, 380], start=180, end=270, fill=stone_light, width=6)
    draw.arc([100, 310, 260, 470], start=180, end=270, fill=stone_light, width=6)
    draw.arc([540, 220, 700, 380], start=270, end=360, fill=stone_light, width=6)
    draw.arc([540, 310, 700, 470], start=270, end=360, fill=stone_light, width=6)
    # Pier pinnacles
    draw.polygon([(75, 220), (97, 150), (120, 220)], fill=stone_mid)
    draw.polygon([(680, 220), (702, 150), (725, 220)], fill=stone_mid)

    # Main Westwork facade body
    draw.rectangle([250, 170, 550, 510], fill=stone_dark, outline=stone_mid, width=1)

    # Twin Towers
    draw.rectangle([140, 120, 260, 510], fill=stone_mid, outline=stone_dark, width=1)
    draw.rectangle([540, 120, 660, 510], fill=stone_mid, outline=stone_dark, width=1)

    # Tower Belfry Lancet Windows (dual tall pointed openings)
    for bx in [170, 210, 570, 610]:
        draw.rectangle([bx - 12, 150, bx + 12, 230], fill=(15, 18, 28, 255))
        draw.polygon([(bx - 12, 150), (bx, 132), (bx + 12, 150)], fill=(15, 18, 28, 255))
        draw.line([(bx, 134), (bx, 230)], fill=stone_light, width=1)

    # Octagonal Spire Pyramids
    draw.polygon([(140, 120), (200, 30), (260, 120)], fill=stone_dark, outline=stone_light, width=1)
    draw.polygon([(540, 120), (600, 30), (660, 120)], fill=stone_dark, outline=stone_light, width=1)
    # Crockets on spires
    for sy in range(45, 115, 14):
        draw.polygon([(190 - (sy - 30) * 0.7, sy), (184 - (sy - 30) * 0.7, sy - 4), (192 - (sy - 30) * 0.7, sy + 3)], fill=stone_light)
        draw.polygon([(210 + (sy - 30) * 0.7, sy), (216 + (sy - 30) * 0.7, sy - 4), (208 + (sy - 30) * 0.7, sy + 3)], fill=stone_light)
        draw.polygon([(590 - (sy - 30) * 0.7, sy), (584 - (sy - 30) * 0.7, sy - 4), (592 - (sy - 30) * 0.7, sy + 3)], fill=stone_light)
        draw.polygon([(610 + (sy - 30) * 0.7, sy), (616 + (sy - 30) * 0.7, sy - 4), (608 + (sy - 30) * 0.7, sy + 3)], fill=stone_light)

    # High Nave Pointed Gable
    draw.polygon([(250, 195), (400, 135), (550, 195)], fill=stone_mid, outline=stone_light, width=1)
    # Crowning cross finial
    draw.line([(400, 115), (400, 135)], fill=(245, 158, 11, 255), width=3)
    draw.line([(393, 122), (407, 122)], fill=(245, 158, 11, 255), width=3)

    # Great Rose Window (Center 400, 265, Radius 62)
    rcx, rcy, rr = 400, 265, 62
    draw.ellipse([rcx - rr, rcy - rr, rcx + rr, rcy + rr], fill=(18, 22, 36, 255), outline=stone_light, width=3)
    # Stained glass radiating segments
    num_spokes = 12
    colors_glass = [(220, 38, 38, 220), (37, 99, 235, 220), (245, 158, 11, 220), (147, 51, 234, 220)]
    for i in range(num_spokes):
        ang1 = math.radians(i * (360 / num_spokes))
        ang2 = math.radians((i + 1) * (360 / num_spokes))
        col = colors_glass[i % len(colors_glass)]
        pts = [
            (rcx, rcy),
            (rcx + int((rr - 6) * math.cos(ang1)), rcy + int((rr - 6) * math.sin(ang1))),
            (rcx + int((rr - 6) * math.cos(ang2)), rcy + int((rr - 6) * math.sin(ang2)))
        ]
        draw.polygon(pts, fill=col)
    # Concentric stone foils and hub
    draw.ellipse([rcx - 42, rcy - 42, rcx + 42, rcy + 42], outline=stone_light, width=2)
    draw.ellipse([rcx - 22, rcy - 22, rcx + 22, rcy + 22], outline=stone_light, width=2)
    draw.ellipse([rcx - 10, rcy - 10, rcx + 10, rcy + 10], fill=stone_mid, outline=(255, 255, 255, 200), width=1)
    for i in range(num_spokes):
        ang = math.radians(i * (360 / num_spokes))
        draw.line([(rcx, rcy), (rcx + int(rr * math.cos(ang)), rcy + int(rr * math.sin(ang)))], fill=stone_light, width=2)

    # Gallery of Kings frieze band
    draw.rectangle([250, 335, 550, 365], fill=stone_mid, outline=stone_dark, width=1)
    for kx in range(265, 545, 18):
        draw.ellipse([kx - 4, 342, kx + 4, 358], fill=(180, 190, 210, 255))

    # Triple Pointed Arch Portals
    # Central Portal
    draw.rectangle([330, 420, 470, 510], fill=(12, 14, 22, 255), outline=stone_light, width=2)
    draw.polygon([(330, 420), (400, 365), (470, 420)], fill=(12, 14, 22, 255), outline=stone_light)
    draw.line([(400, 365), (400, 510)], fill=stone_light, width=2)
    # Concentric archivolts
    draw.polygon([(320, 422), (400, 355), (480, 422)], outline=stone_light, width=1)
    # Lateral Portals
    for px in [195, 605]:
        draw.rectangle([px - 38, 440, px + 38, 510], fill=(12, 14, 22, 255), outline=stone_light, width=2)
        draw.polygon([(px - 38, 440), (px, 395), (px + 38, 440)], fill=(12, 14, 22, 255), outline=stone_light)

    return Image.alpha_composite(base, layer)


def draw_style_03_renaissance(base: Image.Image) -> Image.Image:
    """Style 03: Italian Renaissance (Brunelleschi Loggia & Florence Dome)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    terracotta = (180, 83, 9, 255)
    limestone = (235, 225, 205, 255)
    pietra_serena = (100, 116, 139, 255)
    shadow_col = (50, 40, 35, 200)

    # 1. Ground Loggia (Brunelleschi Arcade)
    draw.rectangle([130, 380, 670, 505], fill=limestone, outline=pietra_serena, width=1)
    # Rustication horizontal ashlar joints
    for yj in range(390, 505, 12):
        draw.line([(130, yj), (670, yj)], fill=(200, 190, 170, 255), width=1)

    # 5 Semicircular Round Arches on Columns
    arch_centers = [184, 292, 400, 508, 616]
    arch_r = 38
    for ac in arch_centers:
        # Shadow recess inside arch
        draw.rectangle([ac - arch_r, 415, ac + arch_r, 505], fill=(45, 35, 30, 255))
        draw.pieslice([ac - arch_r, 415 - arch_r, ac + arch_r, 415 + arch_r], 180, 360, fill=(45, 35, 30, 255))
        # Arch moulding ring
        draw.arc([ac - arch_r, 415 - arch_r, ac + arch_r, 415 + arch_r], 180, 360, fill=pietra_serena, width=3)
        # Keystone
        draw.polygon([(ac - 4, 372), (ac + 4, 372), (ac + 3, 382), (ac - 3, 382)], fill=pietra_serena)
        # Terracotta Medallion (Tondo) in spandrel
        if ac < 600:
            spandrel_x = ac + 54
            draw.ellipse([spandrel_x - 10, 388, spandrel_x + 10, 408], fill=(37, 99, 235, 255), outline=limestone, width=2)
            draw.ellipse([spandrel_x - 5, 393, spandrel_x + 5, 403], fill=(255, 255, 255, 255))

    # Columns supporting arches
    for ac in arch_centers:
        draw.rectangle([ac - arch_r - 4, 415, ac - arch_r + 4, 505], fill=pietra_serena)
        draw.rectangle([ac + arch_r - 4, 415, ac + arch_r + 4, 505], fill=pietra_serena)

    # 2. Piano Nobile (Second Floor)
    draw.rectangle([160, 265, 640, 380], fill=(245, 238, 222, 255), outline=pietra_serena, width=1)
    # Entablature divider
    draw.rectangle([140, 372, 660, 382], fill=pietra_serena)

    # 5 Windows with alternating pediments
    for idx, ac in enumerate(arch_centers):
        wx1, wy1, wx2, wy2 = ac - 16, 290, ac + 16, 355
        draw.rectangle([wx1, wy1, wx2, wy2], fill=(30, 41, 59, 255), outline=pietra_serena, width=2)
        # Window muntins
        draw.line([(ac, wy1), (ac, wy2)], fill=limestone, width=1)
        draw.line([(wx1, wy1 + 30), (wx2, wy1 + 30)], fill=limestone, width=1)
        # Alternating Pediments: Odd = Triangular, Even = Segmental (curved)
        if idx % 2 == 0:
            draw.polygon([(wx1 - 5, wy1), (ac, wy1 - 16), (wx2 + 5, wy1)], fill=limestone, outline=pietra_serena, width=1)
        else:
            draw.chord([wx1 - 6, wy1 - 24, wx2 + 6, wy1 + 8], 180, 360, fill=limestone, outline=pietra_serena, width=1)

    # Balustrade Terrace Roofline
    draw.rectangle([170, 255, 630, 265], fill=pietra_serena)
    for bx in range(175, 630, 10):
        draw.rectangle([bx, 257, bx + 4, 265], fill=limestone)

    # 3. Monumental Octagonal Dome (Santa Maria del Fiore)
    # Drum (Tamburo)
    draw.rectangle([280, 195, 520, 255], fill=limestone, outline=pietra_serena, width=1)
    # 4 Circular Oculi windows in drum
    for ox in [315, 370, 430, 485]:
        draw.ellipse([ox - 14, 212, ox + 14, 240], fill=(30, 41, 59, 255), outline=pietra_serena, width=2)

    # Pointed Octagonal Dome Envelope
    dome_pts = [
        (280, 195),
        (315, 140),
        (365, 105),
        (435, 105),
        (485, 140),
        (520, 195)
    ]
    draw.polygon(dome_pts, fill=terracotta, outline=(140, 50, 0, 255), width=2)
    # Left sunlight, right shadow
    draw.polygon([(280, 195), (315, 140), (365, 105), (400, 105), (400, 195)], fill=(255, 150, 50, 70))
    draw.polygon([(400, 105), (435, 105), (485, 140), (520, 195), (400, 195)], fill=(100, 30, 0, 90))

    # 8 White Marble Radiating Ribs
    rib_xs = [(280, 365), (325, 380), (370, 395), (400, 400), (430, 405), (475, 420), (520, 435)]
    for rx1, rx2 in rib_xs:
        draw.line([(rx1, 195), (rx2, 105)], fill=(255, 255, 255, 230), width=3)

    # 4. Marble Lantern Temple
    draw.rectangle([372, 65, 428, 105], fill=limestone, outline=pietra_serena, width=1)
    for lx in [380, 392, 408, 420]:
        draw.line([(lx, 68), (lx, 102)], fill=pietra_serena, width=2)
    # Conical cap & cross
    draw.polygon([(368, 65), (400, 46), (432, 65)], fill=limestone, outline=pietra_serena)
    draw.ellipse([394, 38, 406, 50], fill=(217, 119, 6, 255))
    draw.line([(400, 30), (400, 42)], fill=(217, 119, 6, 255), width=2)
    draw.line([(395, 34), (405, 34)], fill=(217, 119, 6, 255), width=2)

    return Image.alpha_composite(base, layer)


def draw_style_04_baroque(base: Image.Image) -> Image.Image:
    """Style 04: Baroque (San Carlo alle Quattro Fontane - Undulating Facade)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    gold = (217, 119, 6, 255)
    stone_gold = (210, 185, 140, 255)
    stone_dark = (70, 55, 40, 255)

    # Golden diagonal spotlight beam in background
    spotlight = Image.new("RGBA", base.size, (0, 0, 0, 0))
    sp_draw = ImageDraw.Draw(spotlight)
    sp_draw.polygon([(100, 0), (500, 0), (700, 600), (250, 600)], fill=(245, 158, 11, 45))
    base = Image.alpha_composite(base, spotlight)

    # 1. Undulating Concave-Convex Lower Facade (Borromini)
    # Left Concave Bay
    draw.polygon([(140, 340), (300, 340), (280, 505), (140, 505)], fill=(120, 100, 75, 255), outline=stone_dark)
    # Central Convex Bay (Juts aggressively forward into light)
    draw.polygon([(290, 330), (510, 330), (520, 505), (280, 505)], fill=stone_gold, outline=stone_dark)
    # Right Concave Bay
    draw.polygon([(500, 340), (660, 340), (660, 505), (510, 505)], fill=(140, 120, 90, 255), outline=stone_dark)

    # Colossal Paired Giant Columns with broken entablatures
    column_pairs = [(165, 195), (285, 315), (485, 515), (605, 635)]
    for c1, c2 in column_pairs:
        for cx in [c1, c2]:
            draw.rectangle([cx - 8, 335, cx + 8, 505], fill=(240, 220, 180, 255), outline=stone_dark, width=1)
            # Corinthian Acanthus capital
            draw.polygon([(cx - 12, 325), (cx + 12, 325), (cx + 8, 335), (cx - 8, 335)], fill=gold)
            draw.ellipse([cx - 6, 322, cx + 6, 330], fill=gold)
        # Broken entablature section jutting forward
        draw.rectangle([c1 - 12, 315, c2 + 12, 328], fill=stone_gold, outline=stone_dark, width=1)

    # Central Portal with Oval niche and sculpture
    draw.rectangle([365, 410, 435, 505], fill=(30, 22, 15, 255))
    draw.chord([365, 375, 435, 445], 180, 360, fill=(30, 22, 15, 255), outline=gold, width=2)
    # Gilded Baroque statue in niche
    draw.ellipse([394, 420, 406, 440], fill=gold)
    draw.polygon([(390, 440), (410, 440), (400, 480)], fill=(245, 220, 160, 255))

    # 2. Upper Story & Broken Segmental Pediment
    draw.rectangle([210, 190, 590, 315], fill=stone_gold, outline=stone_dark, width=1)
    # Volute scrolls flanking upper tier
    draw.chord([155, 220, 225, 315], 0, 180, fill=stone_gold, outline=stone_dark, width=2)
    draw.chord([575, 220, 645, 315], 0, 180, fill=stone_gold, outline=stone_dark, width=2)

    # Dramatic Broken Segmental Pediment (Cornice splits at center)
    draw.chord([200, 155, 370, 215], 180, 360, fill=stone_gold, outline=stone_dark, width=3)
    draw.chord([430, 155, 600, 215], 180, 360, fill=stone_gold, outline=stone_dark, width=3)

    # Gilded Oval Cartouche held by angels
    draw.ellipse([360, 175, 440, 255], fill=(35, 25, 18, 255), outline=gold, width=3)
    draw.ellipse([372, 187, 428, 243], fill=gold)

    # 3. Oval Cupola & Radiant Sunburst Glory Rays
    # 16 Radiant Golden Rays
    ray_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    ray_draw = ImageDraw.Draw(ray_layer)
    for i in range(18):
        ang = math.radians(i * 20 - 170)
        rx = 400 + int(120 * math.cos(ang))
        ry = 135 + int(90 * math.sin(ang))
        ray_draw.line([(400, 135), (rx, ry)], fill=(245, 158, 11, 140), width=2)
    layer = Image.alpha_composite(layer, ray_layer)

    # Oval Dome & Cross
    draw.chord([330, 100, 470, 180], 180, 360, fill=stone_gold, outline=stone_dark, width=2)
    draw.line([(400, 72), (400, 102)], fill=gold, width=4)
    draw.line([(390, 82), (410, 82)], fill=gold, width=4)

    return Image.alpha_composite(base, layer)


def draw_style_05_art_deco(base: Image.Image) -> Image.Image:
    """Style 05: Art Deco (Chrysler Building Ziggurat & Sunburst Spire)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    chrome = (220, 225, 235, 255)
    limestone = (180, 190, 205, 255)
    dark_glass = (20, 28, 42, 255)
    amber_glow = (245, 158, 11, 255)

    # 1. Stepped Ziggurat Profile (Tapering Setbacks)
    # Level 1 Base
    draw.rectangle([160, 425, 640, 515], fill=limestone, outline=chrome, width=1)
    draw.rectangle([155, 422, 645, 427], fill=chrome)  # Chrome trim band
    # Level 2 Setback
    draw.rectangle([215, 335, 585, 425], fill=limestone, outline=chrome, width=1)
    draw.rectangle([210, 332, 590, 337], fill=chrome)
    # Level 3 Setback
    draw.rectangle([270, 250, 530, 335], fill=limestone, outline=chrome, width=1)
    draw.rectangle([265, 247, 535, 252], fill=chrome)
    # Level 4 Setback
    draw.rectangle([320, 175, 480, 250], fill=limestone, outline=chrome, width=1)
    draw.rectangle([315, 172, 485, 177], fill=chrome)

    # Projecting Chrome Eagle Wing Ornaments at corners
    for ex in [215, 585]:
        draw.polygon([(ex, 335), (ex - 22 if ex < 400 else ex + 22, 320), (ex, 325)], fill=chrome)

    # Vertical Fluted Piers & Black Glass Spandrels
    for px in range(180, 625, 26):
        draw.line([(px, 425), (px, 515)], fill=(120, 130, 150, 255), width=2)
    for px in range(235, 570, 24):
        draw.line([(px, 335), (px, 425)], fill=(120, 130, 150, 255), width=2)
    for px in range(285, 515, 22):
        draw.line([(px, 250), (px, 335)], fill=(120, 130, 150, 255), width=2)
    for px in range(335, 470, 20):
        draw.line([(px, 175), (px, 250)], fill=(120, 130, 150, 255), width=2)

    # Chevrons & Geometric fountain friezes
    for cx in range(285, 510, 30):
        draw.line([(cx, 328), (cx + 10, 320), (cx + 20, 328)], fill=amber_glow, width=2)

    # 2. Polished Sunburst Spire (Chrysler Motif: 4 nested parabolic steel arches)
    arch_tiers = [
        (330, 470, 175, 145),
        (345, 455, 145, 120),
        (360, 440, 120, 95),
        (375, 425, 95, 75)
    ]
    for x1, x2, y_bot, y_top in arch_tiers:
        # Parabolic chrome arch
        draw.chord([x1, y_top - (y_bot - y_top), x2, y_bot], 180, 360, fill=(230, 235, 245, 255), outline=chrome, width=2)
        # Radiating triangular cutouts (sunburst segments) glowing with amber light
        mid_x = (x1 + x2) // 2
        for sx in range(x1 + 10, x2 - 10, 16):
            draw.polygon([(sx, y_bot - 4), (sx + 8, y_bot - 4), (mid_x, y_top + 6)], fill=amber_glow)

    # 3. Stainless Steel Needle Mast
    draw.polygon([(396, 75), (404, 75), (401, 30), (399, 30)], fill=chrome)

    # 4. Ruby Aircraft Warning Beacon
    beacon_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    b_draw = ImageDraw.Draw(beacon_layer)
    b_draw.ellipse([385, 20, 415, 50], fill=(239, 68, 68, 60))
    b_draw.ellipse([392, 27, 408, 43], fill=(239, 68, 68, 140))
    b_draw.ellipse([397, 32, 403, 38], fill=(255, 255, 255, 255))
    layer = Image.alpha_composite(layer, beacon_layer)

    return Image.alpha_composite(base, layer)


def draw_style_06_bauhaus(base: Image.Image) -> Image.Image:
    """Style 06: Bauhaus & International Style (Gropius Glass Workshop)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    bauhaus_red = (239, 68, 68, 255)
    bauhaus_yellow = (251, 191, 36, 255)
    bauhaus_blue = (37, 99, 235, 255)
    stucco_white = (255, 255, 255, 255)
    steel_dark = (30, 41, 59, 255)
    glass_fill = (191, 219, 254, 180)

    # 1. Asymmetric Left Wing (Solid White Stucco Lab Block)
    draw.rectangle([120, 190, 230, 445], fill=stucco_white, outline=steel_dark, width=2)
    # Razor-sharp shadow on left edge
    draw.rectangle([120, 190, 135, 445], fill=(210, 218, 226, 255))
    # Crisp horizontal ribbon window
    draw.rectangle([135, 260, 230, 280], fill=steel_dark)
    draw.rectangle([135, 330, 230, 350], fill=steel_dark)

    # Glazed Staircase Tower with primary Bauhaus Red framing
    draw.rectangle([210, 160, 245, 445], fill=(147, 197, 253, 140), outline=bauhaus_red, width=3)
    # Diagonal staircase lines visible through glass
    for sy in range(180, 430, 35):
        draw.line([(212, sy), (243, sy + 25)], fill=bauhaus_red, width=2)

    # 2. Continuous Glass Curtain Wall Floating Pavilion
    # Ground Floor Recessed Cylindrical Pilotis (columns)
    for px in [280, 370, 460, 550, 640]:
        draw.rectangle([px - 8, 420, px + 8, 485], fill=steel_dark)

    # Main Floating Glass Box
    draw.rectangle([230, 190, 680, 420], fill=glass_fill, outline=steel_dark, width=2)

    # Exposed Interior Floor Slabs (visible through curtain wall)
    draw.rectangle([230, 265, 680, 275], fill=(71, 85, 105, 200))
    draw.rectangle([230, 340, 680, 350], fill=(71, 85, 105, 200))

    # Steel Curtain Wall Grid
    # Vertical mullions every 30 px
    for mx in range(260, 680, 30):
        draw.line([(mx, 190), (mx, 420)], fill=steel_dark, width=2)
    # Horizontal transoms every 38 px
    for ty in range(228, 420, 38):
        draw.line([(230, ty), (680, ty)], fill=steel_dark, width=1)

    # 3. Primary Color Accents (De Stijl Palette)
    # Bauhaus Red: Entrance canopy
    draw.rectangle([220, 445, 290, 452], fill=bauhaus_red)
    draw.line([(220, 445), (220, 485)], fill=steel_dark, width=2)
    # Bauhaus Yellow: Cantilevered roof sun-blade
    draw.rectangle([240, 175, 480, 183], fill=bauhaus_yellow)
    # Bauhaus Blue: Functional ground service block
    draw.rectangle([480, 425, 660, 485], fill=bauhaus_blue, outline=steel_dark, width=1)

    # 4. Flat Roof with slim tubular black steel safety railings
    draw.line([(120, 185), (680, 185)], fill=steel_dark, width=2)
    for rx in range(130, 680, 25):
        draw.line([(rx, 185), (rx, 190)], fill=steel_dark, width=1)

    return Image.alpha_composite(base, layer)


def draw_style_07_brutalism(base: Image.Image) -> Image.Image:
    """Style 07: Brutalism (Boston City Hall Inverted Cantilever Megastructure)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    concrete_slate = (100, 116, 139, 255)
    concrete_light = (148, 163, 184, 255)
    concrete_dark = (30, 41, 59, 255)
    shadow_band = (15, 23, 42, 255)

    # Left Flanking Windowless Monolithic Elevator Core Bunker (with 45° chamfer)
    draw.polygon([(110, 135), (175, 135), (190, 150), (190, 495), (110, 495)], fill=(75, 85, 100, 255), outline=shadow_band, width=2)
    # Vertical fluting grooves on service bunker
    for bx in [130, 150, 170]:
        draw.line([(bx, 140), (bx, 490)], fill=concrete_dark, width=2)

    # 1. Inverted Stepped Cantilever Massing
    # Lower Core with angled concrete pilotis
    draw.rectangle([270, 415, 530, 495], fill=concrete_dark, outline=shadow_band, width=2)
    for px in [300, 370, 430, 500]:
        # Heavy 60-degree angled concrete pylons
        draw.polygon([(px - 14, 495), (px + 14, 495), (px + 24, 415), (px - 4, 415)], fill=concrete_slate, outline=shadow_band)

    # Mid Tier Cantilever (Overhangs lower tier)
    draw.rectangle([190, 305, 610, 415], fill=concrete_slate, outline=shadow_band, width=2)
    # Heavy shadow beneath mid tier
    draw.rectangle([270, 415, 530, 425], fill=shadow_band)

    # Mammoth Upper Tier Cantilever (Aggressively overhangs mid tier)
    draw.rectangle([130, 165, 670, 305], fill=concrete_light, outline=shadow_band, width=2)
    # Massive shadow beneath mammoth upper tier
    draw.rectangle([190, 305, 610, 318], fill=shadow_band)

    # 2. Textured Béton Brut (Board-marked formwork striations)
    for y in range(175, 305, 10):
        draw.line([(132, y), (668, y)], fill=(120, 135, 155, 255), width=1)
    for y in range(315, 415, 10):
        draw.line([(192, y), (608, y)], fill=(80, 95, 115, 255), width=1)

    # Shuttering circular tie-rod holes
    for x in range(150, 660, 45):
        for y in [185, 235, 285]:
            draw.ellipse([x - 2, y - 2, x + 2, y + 2], fill=shadow_band)
            draw.point((x - 1, y - 1), fill=(255, 255, 255, 180))

    # 3. Deeply Recessed Brise-Soleil Window Slots with dramatic cast shadows
    for wx in range(160, 650, 36):
        # Angled concrete fin
        draw.polygon([(wx, 195), (wx + 8, 195), (wx + 4, 275), (wx - 4, 275)], fill=concrete_slate)
        # Deep window slot with diagonal cast shadow wedge
        draw.rectangle([wx + 8, 205, wx + 24, 265], fill=(15, 20, 30, 255))
        draw.polygon([(wx + 8, 205), (wx + 24, 205), (wx + 8, 235)], fill=(5, 8, 15, 255))

    # Mid-tier window slots
    for wx in range(215, 595, 32):
        draw.rectangle([wx, 335, wx + 18, 385], fill=(15, 20, 30, 255))
        draw.polygon([(wx, 335), (wx + 18, 335), (wx, 360)], fill=(5, 8, 15, 255))

    return Image.alpha_composite(base, layer)


def draw_style_08_hightech(base: Image.Image) -> Image.Image:
    """Style 08: High-Tech Architecture (Pompidou / Lloyd's Exoskeleton)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    steel_blue = (59, 130, 246, 255)
    duct_blue = (37, 99, 235, 255)
    pipe_green = (16, 185, 129, 255)
    tray_yellow = (245, 158, 11, 255)
    truss_red = (239, 68, 68, 255)
    steel_chrome = (203, 213, 225, 255)
    glass_dark = (15, 23, 42, 240)

    # Main building envelope behind exoskeleton
    draw.rectangle([180, 160, 620, 490], fill=glass_dark, outline=(51, 65, 85, 255), width=2)
    # Floor slab bands
    floor_ys = [190, 270, 350, 430]
    for fy in floor_ys:
        draw.rectangle([180, fy - 6, 620, fy + 6], fill=(51, 65, 85, 255))
        # Yellow electrical conduit tray
        draw.line([(180, fy + 8), (620, fy + 8)], fill=tray_yellow, width=3)

    # 1. Exposed Tubular Steel Exoskeleton
    # Mega-Columns at X=150 and X=650
    for cx in [150, 650]:
        # Gradient tubular column
        draw.rectangle([cx - 10, 120, cx + 10, 495], fill=steel_chrome, outline=(100, 116, 139, 255), width=1)
        # Ring joint collars every 60 px
        for cy in range(130, 495, 60):
            draw.rectangle([cx - 13, cy - 3, cx + 13, cy + 3], fill=(148, 163, 184, 255), outline=(51, 65, 85, 255))

    # Cast-Steel Gerberette Cantilever Brackets extending from mega-columns
    for cx, sign in [(150, 1), (650, -1)]:
        for fy in floor_ys:
            # Gerberette pointed cantilever
            draw.polygon([(cx, fy - 8), (cx + sign * 35, fy - 2), (cx + sign * 35, fy + 2), (cx, fy + 8)], fill=(148, 163, 184, 255), outline=steel_chrome)
            # Tension anchor tie rods
            draw.line([(cx + sign * 35, fy), (cx + sign * 35, fy + 80 if fy < 430 else 495)], fill=steel_chrome, width=2)

    # Giant Tubular X-Bracing
    for i in range(len(floor_ys) - 1):
        y1, y2 = floor_ys[i], floor_ys[i + 1]
        draw.line([(185, y1), (615, y2)], fill=steel_chrome, width=3)
        draw.line([(185, y2), (615, y1)], fill=steel_chrome, width=3)
        # Cast spherical connector pin at center intersection
        mid_y = (y1 + y2) // 2
        draw.ellipse([394, mid_y - 6, 406, mid_y + 6], fill=(226, 232, 240, 255), outline=(71, 85, 105, 255))

    # 2. Color-Coded Mechanical Systems (Pompidou Syntax)
    # Blue Air Ducts (large conduits)
    for dy in [200, 600]:
        draw.rectangle([dy - 8, 140, dy + 8, 490], fill=duct_blue, outline=(29, 78, 216, 255), width=1)
        for j in range(150, 490, 40):
            draw.rectangle([dy - 10, j - 2, dy + 10, j + 2], fill=(96, 165, 250, 255))

    # Green Fluid/Water Pipes with 90° elbows
    for px in [230, 570]:
        draw.line([(px, 150), (px, 490)], fill=pipe_green, width=6)
        draw.ellipse([px - 5, 245, px + 5, 255], fill=(52, 211, 153, 255))
        draw.ellipse([px - 5, 325, px + 5, 335], fill=(52, 211, 153, 255))

    # 3. Diagonal Glazed Escalator Caterpillar
    # Slices from bottom-left (165, 470) to top-right (635, 190)
    steps = 15
    for s in range(steps):
        t1 = s / steps
        t2 = (s + 1) / steps
        x1 = int(165 + (635 - 165) * t1)
        y1 = int(470 + (190 - 470) * t1)
        x2 = int(165 + (635 - 165) * t2)
        y2 = int(470 + (190 - 470) * t2)
        # Red structural truss under tube
        draw.line([(x1, y1 + 10), (x2, y2 + 10)], fill=truss_red, width=4)
        # Glass tube section
        draw.polygon([(x1, y1 - 8), (x2, y2 - 8), (x2, y2 + 8), (x1, y1 + 8)], fill=(147, 197, 253, 160), outline=(255, 255, 255, 200))
        # Illuminated pedestrian tread
        draw.ellipse([x1 - 3, y1 - 3, x1 + 3, y1 + 3], fill=tray_yellow)

    # 4. Rooftop Plant & Crane Maintenance Gantry
    draw.rectangle([220, 115, 580, 140], fill=(71, 85, 105, 255), outline=steel_chrome, width=1)
    # Cooling towers
    for tx in [260, 320]:
        draw.ellipse([tx - 12, 100, tx + 12, 120], fill=steel_chrome)
    # Mobile yellow crane rail
    draw.line([(380, 100), (520, 100)], fill=tray_yellow, width=4)
    draw.polygon([(460, 85), (475, 100), (445, 100)], fill=tray_yellow)

    return Image.alpha_composite(base, layer)


def draw_style_09_deconstructivism(base: Image.Image) -> Image.Image:
    """Style 09: Deconstructivism (Guggenheim Bilbao Titanium Fragments)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    titanium_light = (228, 228, 231, 255)
    titanium_mid = (161, 161, 170, 255)
    zinc_dark = (113, 113, 122, 255)
    amber_glow = (253, 230, 138, 255)
    shadow_deep = (9, 9, 11, 220)

    # 1. Deep Pitch-Black Geometric Shadow Wedges
    shadow_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    sh_draw = ImageDraw.Draw(shadow_layer)
    sh_draw.polygon([(180, 485), (280, 485), (240, 380)], fill=shadow_deep)
    sh_draw.polygon([(320, 485), (510, 485), (420, 350)], fill=shadow_deep)
    sh_draw.polygon([(470, 475), (640, 485), (580, 360)], fill=shadow_deep)
    layer = Image.alpha_composite(layer, shadow_layer)

    # 2. Colliding Non-Orthogonal Fragmented Polygons
    # Fragment 1: Skyward Knife Wedge (Acute titanium silver blade)
    frag1 = [(170, 485), (110, 220), (270, 150), (310, 380)]
    draw.polygon(frag1, fill=titanium_light, outline=(255, 255, 255, 255), width=2)
    # Shading facet
    draw.polygon([(110, 220), (270, 150), (210, 330)], fill=(244, 244, 245, 255))
    # Knife-edge razor highlight
    draw.line([(110, 220), (270, 150)], fill=(255, 255, 255, 255), width=3)

    # Fragment 2: Tilted Monolith Prism (Leaning violently rightwards)
    frag2 = [(280, 485), (320, 270), (480, 125), (510, 435)]
    draw.polygon(frag2, fill=zinc_dark, outline=(63, 63, 70, 255), width=2)
    # Split facet on tilted monolith
    draw.polygon([(320, 270), (480, 125), (420, 320)], fill=titanium_mid)

    # Fragment 3: Fractured Glass Wedge
    frag3 = [(470, 475), (530, 250), (690, 180), (640, 485)]
    draw.polygon(frag3, fill=(165, 243, 252, 170), outline=(255, 255, 255, 220), width=2)
    # Interior structural diagonal struts in glass
    draw.line([(470, 475), (690, 180)], fill=(71, 85, 105, 200), width=2)
    draw.line([(530, 250), (640, 485)], fill=(71, 85, 105, 200), width=2)

    # 3. Billowing Titanium Fish-Scale Canopy (Bilbao Motif)
    # Undulating curved metallic sheets
    canopy_pts = [
        (250, 160),
        (330, 110),
        (420, 145),
        (510, 95),
        (560, 140),
        (480, 180),
        (380, 150),
        (290, 190)
    ]
    draw.polygon(canopy_pts, fill=(212, 212, 216, 255), outline=(255, 255, 255, 255), width=2)
    # Metallic fish-scale ridges
    for cx in range(280, 520, 25):
        draw.arc([cx - 15, 120, cx + 15, 160], start=30, end=150, fill=(255, 255, 255, 200), width=2)

    # 4. Libeskind Diagonal Slash Window Incisions with incandescent amber glow
    slashes = [
        ((160, 320), (220, 280)),
        ((190, 410), (250, 380)),
        ((350, 380), (430, 340)),
        ((370, 290), (440, 250)),
        ((410, 210), (460, 180)),
    ]
    for p1, p2 in slashes:
        draw.line([p1, p2], fill=amber_glow, width=5)
        draw.line([p1, p2], fill=(255, 255, 255, 255), width=2)

    return Image.alpha_composite(base, layer)


def draw_style_10_parametric(base: Image.Image) -> Image.Image:
    """Style 10: Parametric & Organic (Zaha Hadid Biomorphic Voronoi Shell)"""
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    shell_white = (248, 250, 252, 255)
    shell_mid = (203, 213, 225, 255)
    shell_shade = (148, 163, 184, 255)
    aperture_glow = (56, 189, 248, 255)
    aperture_deep = (3, 105, 161, 255)

    # 1. Reflective Water Basin & Ripple (Ground plane at Y=[490, 520])
    draw.rectangle([80, 490, 720, 520], fill=(12, 32, 55, 255), outline=(56, 189, 248, 120), width=1)
    for rx in range(120, 680, 35):
        draw.arc([rx - 25, 498, rx + 25, 512], start=0, end=180, fill=(56, 189, 248, 80), width=1)

    # 2. Continuous Double-Curved Biomorphic Fluid Shell (Sine / Catenary Spline)
    # Modeled via multi-segmented parametric ribbon isocurves
    steps = 60
    spine_pts = []
    for s in range(steps + 1):
        t = s / steps
        # X: 110 to 690
        x = 110 + (690 - 110) * t
        # Y: Complex fluid catenary with two peaks and a central saddle
        # Peak 1 at t=0.28 (X~270, Y~180), Saddle at t=0.54 (X~420, Y~290), Peak 2 at t=0.82 (X~580, Y~145)
        wave = math.sin(t * math.pi) ** 1.2
        dip = math.sin(t * math.pi * 2.0) * 85.0
        y = int(495 - wave * 310 + dip)
        spine_pts.append((x, y))

    # Base shell envelope polygon
    shell_polygon = [(110, 495)] + spine_pts + [(690, 495)]
    draw.polygon(shell_polygon, fill=shell_white, outline=shell_shade, width=2)

    # Multi-pass isocurve ribbons (emphasizing curvature)
    for offset in [15, 30, 48, 68]:
        ribbon_pts = []
        for x, y in spine_pts:
            ny = min(495, y + offset)
            ribbon_pts.append((x, ny))
        draw.line(ribbon_pts, fill=(180, 195, 215, 180), width=1)

    # Under-belly soft occlusion shading
    occlusion_pts = [(spine_pts[i][0], min(495, spine_pts[i][1] + 45)) for i in range(len(spine_pts))]
    draw.polygon(occlusion_pts + [(690, 495), (110, 495)], fill=(15, 23, 42, 60))

    # 3. Algorithmic Voronoi Apertures (18 organic rounded cell openings glowing cyan)
    aperture_specs = [
        # (cx, cy, rx, ry, rot)
        (220, 260, 14, 22, -25),
        (260, 210, 18, 28, -15),
        (300, 225, 20, 26, -5),
        (340, 260, 17, 24, 10),
        (380, 300, 15, 20, 20),
        (420, 320, 14, 18, 15),
        (460, 290, 16, 22, -10),
        (500, 230, 20, 28, -20),
        (540, 185, 22, 32, -15),
        (580, 175, 20, 30, 5),
        (620, 220, 16, 25, 25),
        # Micro-perforations along lower structural skirt
        (170, 440, 8, 12, 10),
        (210, 390, 9, 14, 15),
        (280, 350, 11, 16, 5),
        (490, 380, 11, 16, -15),
        (560, 330, 12, 18, -10),
        (630, 360, 10, 15, 20),
        (665, 430, 7, 10, 25),
    ]

    for cx, cy, rx, ry, rot in aperture_specs:
        # Glow layer
        draw.ellipse([cx - rx - 2, cy - ry - 2, cx + rx + 2, cy + ry + 2], fill=aperture_glow)
        # Deep aperture core
        draw.ellipse([cx - rx, cy - ry, cx + rx, cy + ry], fill=aperture_deep, outline=(255, 255, 255, 230), width=1)
        # Center turquoise light point
        draw.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], fill=(255, 255, 255, 255))

    # 4. Longitudinal NURBS Contour Flowlines
    for t_step in range(6):
        flow_pts = []
        for s in range(0, steps + 1, 2):
            x, y = spine_pts[s]
            flow_y = y + t_step * 14 + int(math.sin(s * 0.4) * 4)
            if flow_y < 495:
                flow_pts.append((x, flow_y))
        if len(flow_pts) > 2:
            draw.line(flow_pts, fill=(56, 189, 248, 140), width=1)

    return Image.alpha_composite(base, layer)


# ==============================================================================
# 5. Master Asset Generation Engine
# ==============================================================================

STYLE_RENDERERS = {
    1: draw_style_01_classical,
    2: draw_style_02_gothic,
    3: draw_style_03_renaissance,
    4: draw_style_04_baroque,
    5: draw_style_05_art_deco,
    6: draw_style_06_bauhaus,
    7: draw_style_07_brutalism,
    8: draw_style_08_hightech,
    9: draw_style_09_deconstructivism,
    10: draw_style_10_parametric,
}

STYLE_GRADIENTS = {
    # 1. Classical: Mediterranean Azure to Warm Limestone Sunset
    1: [(0.0, (15, 30, 65)), (0.45, (70, 95, 140)), (0.85, (225, 160, 90)), (1.0, (180, 110, 50))],
    # 2. Gothic: Mystical Sapphire & Deep Indigo Twilight
    2: [(0.0, (8, 12, 28)), (0.50, (22, 28, 56)), (1.0, (54, 32, 70))],
    # 3. Renaissance: Florentine Golden Hour Tuscan Ochre
    3: [(0.0, (35, 75, 130)), (0.45, (140, 120, 145)), (0.85, (225, 145, 90)), (1.0, (160, 85, 45))],
    # 4. Baroque: Theatrical Caravaggesque Chiaroscuro Umber
    4: [(0.0, (18, 14, 12)), (0.45, (45, 32, 22)), (0.85, (95, 68, 35)), (1.0, (30, 20, 15))],
    # 5. Art Deco: Machine-Age Gotham Obsidian to Electric Skyscape
    5: [(0.0, (10, 14, 24)), (0.45, (22, 34, 52)), (0.85, (38, 55, 80)), (1.0, (18, 25, 38))],
    # 6. Bauhaus: Studio Neutral Daylight Grey (Modernist Light)
    6: [(0.0, (242, 245, 249)), (0.50, (228, 234, 241)), (1.0, (210, 218, 226))],
    # 7. Brutalism: Atmospheric Overcast Storm Slate
    7: [(0.0, (38, 48, 64)), (0.45, (65, 80, 100)), (0.85, (110, 125, 145)), (1.0, (75, 85, 100))],
    # 8. High-Tech: Midnight Navy to Industrial Electric Cyan
    8: [(0.0, (12, 18, 32)), (0.45, (22, 36, 60)), (0.85, (36, 64, 98)), (1.0, (18, 28, 45))],
    # 9. Deconstructivism: Electric Gunmetal Anthracite Twilight
    9: [(0.0, (18, 18, 22)), (0.45, (30, 32, 38)), (0.85, (48, 52, 60)), (1.0, (22, 24, 28))],
    # 10. Parametric: Deep Navy to Bioluminescent Cyan Dawn
    10: [(0.0, (8, 14, 30)), (0.45, (18, 38, 65)), (0.85, (28, 68, 98)), (1.0, (12, 22, 40))],
}

TAGLINES = {
    1: "Doric Peristyle & Harmonic Proportions",
    2: "Pointed Arches & Radiant Rose Window",
    3: "Brunelleschi Loggia & Catenary Cupola",
    4: "Borromini Undulations & Chiaroscuro",
    5: "Stepped Ziggurat & Sunburst Spire",
    6: "Curtain Wall & Pilotis",
    7: "Béton Brut & Inverted Cantilevers",
    8: "Exposed Exoskeleton & Ducts",
    9: "Titanium Wedges & Deconstructed Slashes",
    10: "Fluid Biomorphic Envelope",
}


def generate_single_asset(
    style: Dict[str, Any],
    category_map: Dict[str, Dict[str, Any]],
    output_dir: Path
) -> Path:
    """Generates a single 800x600 PNG architectural asset."""
    sid = style["id"]
    filename = style["filename"]
    cat_id = style["category_id"]
    category = category_map[cat_id]

    print(f"  Rendering [{sid:02d}] {filename} ({style['style_en']})...")

    # 1. Base gradient
    width, height = 800, 600
    stops = STYLE_GRADIENTS.get(sid, [(0.0, (20, 25, 35)), (1.0, (40, 50, 70))])
    base = create_vertical_gradient(width, height, stops)

    # 2. Blueprint drafting grid layer
    is_light = (sid == 6)  # Bauhaus uses dark grid lines on light grey studio background
    grid = draw_blueprint_grid(width, height, is_light=is_light)
    base = Image.alpha_composite(base, grid)

    # 3. Procedural Vector Silhouette
    renderer = STYLE_RENDERERS.get(sid)
    if renderer:
        base = renderer(base)

    # 4. Translucent Bottom Pill Badge Overlay
    tagline = TAGLINES.get(sid, "Architectural Exemplar")
    base = draw_bottom_pill_badge(
        base=base,
        style_id=sid,
        style_en=style["style_en"],
        style_it=style["style_it"],
        category_name_en=category["name_en"],
        category_color_hex=category["color_hex"],
        tagline=tagline
    )

    # 5. Save as 24-bit RGB PNG
    final_img = base.convert("RGB")
    out_path = output_dir / filename
    final_img.save(out_path, format="PNG", optimize=True)

    size_kb = out_path.stat().st_size / 1024.0
    print(f"      --> Saved {out_path.name} ({size_kb:.1f} KB, 800x600 RGB)")
    return out_path


def load_authoritative_metadata(repo_root: Path) -> Dict[str, Any]:
    """Loads styles.json metadata from shared/data/styles.json."""
    data_file = repo_root / "shared" / "data" / "styles.json"
    if not data_file.exists():
        raise FileNotFoundError(f"Missing authoritative metadata: {data_file}")
    with open(data_file, "r", encoding="utf-8") as f:
        return json.load(f)


def sync_assets_to_platforms(repo_root: Path):
    """
    Syncs generated mock assets to Android and Windows platform directories
    if they exist in the repository tree.
    """
    shared_images = repo_root / "shared" / "images"
    shared_data = repo_root / "shared" / "data"

    # Android target: android/app/src/main/assets/
    android_assets = repo_root / "android" / "app" / "src" / "main" / "assets"
    if android_assets.parent.exists():
        print("  Syncing assets to Android project...")
        android_img = android_assets / "images"
        android_dat = android_assets / "data"
        android_img.mkdir(parents=True, exist_ok=True)
        android_dat.mkdir(parents=True, exist_ok=True)
        for p in shared_images.glob("*.png"):
            shutil.copy2(p, android_img / p.name)
        shutil.copy2(shared_data / "styles.json", android_dat / "styles.json")
        print("    --> Android assets synchronized.")

    # Windows target: windows/assets/
    windows_assets = repo_root / "windows" / "assets"
    if windows_assets.parent.exists():
        print("  Syncing assets to Windows project...")
        win_img = windows_assets / "images"
        win_dat = windows_assets / "data"
        win_img.mkdir(parents=True, exist_ok=True)
        win_dat.mkdir(parents=True, exist_ok=True)
        for p in shared_images.glob("*.png"):
            shutil.copy2(p, win_img / p.name)
        shutil.copy2(shared_data / "styles.json", win_dat / "styles.json")
        print("    --> Windows assets synchronized.")


def main():
    print("================================================================")
    print("  Architecture Profiling — Mock Asset & Metadata Generator      ")
    print("================================================================\n")

    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent if script_dir.name == "scripts" else script_dir

    shared_data_dir = repo_root / "shared" / "data"
    shared_images_dir = repo_root / "shared" / "images"

    shared_data_dir.mkdir(parents=True, exist_ok=True)
    shared_images_dir.mkdir(parents=True, exist_ok=True)

    print(f"Loading metadata from {shared_data_dir / 'styles.json'}...")
    metadata = load_authoritative_metadata(repo_root)

    categories = metadata["categories"]
    styles = metadata["styles"]
    category_map = {c["id"]: c for c in categories}

    # Ensure architecture_images.json is synchronized
    arch_images_copy = shared_data_dir / "architecture_images.json"
    with open(arch_images_copy, "w", encoding="utf-8") as f:
        json.dump(metadata, f, indent=2, ensure_ascii=False)
    print(f"Wrote synchronized compatibility copy: {arch_images_copy.name}")

    print(f"\nGenerating {len(styles)} Architectural Mock Images (800x600 PNG)...")
    for s in styles:
        generate_single_asset(s, category_map, shared_images_dir)

    print("\nChecking optional platform asset synchronization...")
    sync_assets_to_platforms(repo_root)

    print("\n================================================================")
    print("  ASSET GENERATION COMPLETE: 10/10 IMAGES GENERATED CLEANLY     ")
    print("================================================================\n")


if __name__ == "__main__":
    main()
