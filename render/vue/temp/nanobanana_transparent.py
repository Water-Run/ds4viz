from PIL import Image
from collections import deque

# ---------- 参数 ----------
GREEN_DOMINANCE = 20   # 绿通道比 R/B 高多少算绿
# -------------------------

img = Image.open("LOGO_RAW.png").convert("RGBA")
pixels = img.load()
w, h = img.size

visited = [[False]*h for _ in range(w)]
queue = deque()

def is_green(r, g, b):
    return g > r + GREEN_DOMINANCE and g > b + GREEN_DOMINANCE

# 1️⃣ 从四周边界，把“背景绿”入队
for x in range(w):
    for y in (0, h-1):
        r,g,b,a = pixels[x,y]
        if is_green(r,g,b):
            queue.append((x,y))
            visited[x][y] = True

for y in range(h):
    for x in (0, w-1):
        r,g,b,a = pixels[x,y]
        if is_green(r,g,b):
            queue.append((x,y))
            visited[x][y] = True

# 2️⃣ 泛洪：只扩散“背景绿”
while queue:
    x, y = queue.popleft()
    pixels[x,y] = (0,0,0,0)  # 背景 → 透明

    for dx, dy in ((1,0),(-1,0),(0,1),(0,-1)):
        nx, ny = x+dx, y+dy
        if 0 <= nx < w and 0 <= ny < h and not visited[nx][ny]:
            r,g,b,a = pixels[nx,ny]
            if is_green(r,g,b):
                visited[nx][ny] = True
                queue.append((nx,ny))

# 3️⃣ 其余像素全部保留（包括内部噪点）
img.save("cutout.png")