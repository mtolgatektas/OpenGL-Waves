# 🌊 Ocean Waves in OpenGL
*"I just really like how water looks."*

This project is my personal dive into the world of real-time fluid simulation. It started with a simple goal: figure out how to make a flat grid of triangles look and move like the ocean. 

I spent a lot of time with Jerry Tessendorf's paper, [**"Simulating Ocean Water"**](https://people.cs.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf). While I originally wanted to go full-speed with FFT (Fast Fourier Transform) waves, they were a bit too heavy for my hardware to handle in real-time. Instead, I focused on making **Gerstner Waves** look as natural as possible through layered shaders and lighting tricks.

<p align="center">
  <img src="https://github.com/user-attachments/assets/33e4b2ce-7aad-4879-92a8-77e169d77fa1" width="32%" />
  <img src="https://github.com/user-attachments/assets/cbc35f02-4fda-474a-a9ca-ce65ef322220" width="32%" />
  <img src="https://github.com/user-attachments/assets/ed2e918d-c3ed-49d1-a3a8-e694db49784a" width="32%" />
</p>

---

## 🚀 Key Features

### 🌊 Gerstner Wave Geometry
The water isn't just a scrolling texture—it's a $500 \times 500$ mesh grid displaced in real-time. 
* **Layered Waves:** I stacked 4 distinct Gerstner waves. By mixing big swells with tiny "chops" and using prime numbers for the wavelengths, I broke up the repetitive tiling look to create an organic, chaotic surface.
* **Dynamic Normals:** Normals are calculated per-vertex in the shader based on the partial derivatives of the wave functions, ensuring the lighting stays accurate as the geometry deforms.

### 🌌 Skybox & Environment Mapping
The environment plays a huge role in the realism of the water.
* **Cubemap Integration:** A high-resolution skybox provides the background and the source for all surface reflections.
* **Optimized Rendering:** The skybox uses a custom shader that sets depth to the maximum value (`gl_Position = pos.xyww`), ensuring it always renders behind the water mesh without expensive depth sorted calls.
* **Reflection Sampling:** The water shader calculates a reflection vector using the camera position and the surface normal to sample the cubemap in real-time.

### 💡 The Shading Pipeline
* **Fresnel Effect:** Implemented the Fresnel equations to simulate how water is more reflective at grazing angles (the horizon) and more transparent when looking straight down.
* **Sun Glints:** High-exponent specular highlights (Blinn-Phong) create that harsh "shimmer" where the waves catch the light.
* **Perlin Micro-Detail:** I used scrolling **Normal and DuDv maps** to simulate micro-ripples and surface tension that would be too expensive to model with actual triangles.
* **Underwater Post-Processing:** When the camera dips below $y=0$, the shader triggers an underwater mode with exponential fog (murky green/blue tint) and Total Internal Reflection on the water's "ceiling."

---

## 🎮 Controls

| Key | Action |
| :--- | :--- |
| **W, A, S, D** | Fly through the scene |
| **Mouse** | Look around (Mouse Callback) |
| **Up / Down** | Change Wave Amplitude (Height) |
| **Left / Right** | Change Wave Frequency (Speed/Tightness) |
| **L / K** | Toggle Lighting (Physical vs. Raw Mesh) |
| **C / X** | Toggle Height Colors |

---

## 🛠️ Built With
* **C++** & **OpenGL 3.3 (Core Profile)**
* **GLFW / GLEW** — Window management and extension loading.
* **GLM** — All the linear algebra behind the wave math.
* **stb_image** — Used for loading the `.png` and `.jpg` textures for the skybox and normal maps.

*Note: The skybox 'ceiling' texture has a minor seam issue when looking straight up. It's a known UV mapping bug on the list to fix!*

---

## 📚 Sources & Assets
* **Jerry Tessendorf:** ["Simulating Ocean Water"](https://people.cs.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf) — Foundations for the wave math.
* **LearnOpenGL:** Fly-through [camera logic](https://learnopengl.com/Getting-started/Camera) and base skybox structure.
* **iamyoukou/fftWater:** Adapted the cubemap loading method and sourced the environment textures from this [repo](https://github.com/iamyoukou/fftWater/tree/master).
* **czartur/ocean_fft:** Reference for the camera and key control structure in the update loop ([source](https://github.com/czartur/ocean_fft/blob/main/src/cgp_custom.cpp)).

---

## 📝 License
MIT License. Feel free to use the shader logic for your own projects!
