# Introduction
In an attempt to allow for greater artist control and graphical quality within Basilisk Engine, we decided to pursue an implamentation of pricipled shading. In particular, the model is based on the model presented in Burley's 2012 paper, [Physically Based Shading at Disney](https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf). Burley also released another paper in 2015, [Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering](https://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf) which extened the BRDF in the previous model to a BSDF for subsurface scattering. Because the original model includes an approximation for subsurface scattering, we decided not to used the extended version to perserve performance. In addition to this model, we also analyzed the principled shading model [implemented](https://github.com/blender/blender/blob/main/source/blender/gpu/shaders/material/gpu_shader_material_principled.glsl) by Blender. Their [Principled BSDF](https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/principled.html), though a BSDF, was a great tool for seeing what the result of a principled model should look like.

# Terms
It is helpful to establish a number of terms a vectors that will be used in coming sections.

![alt text](image.png)

Where:
- $\vec n$ is the surface normal
- $\vec l$ is the light vector
- $\vec v$ is the view vector
- $\vec h$ is the half vector between $\vec l$ and $\vec v$

Additionally, the following angles between vectors will be used:
- $\theta_l$ is the angle between $\vec n$ and $\vec l$
- $\theta_v$ is the angle between $\vec n$ and $\vec v$
- $\theta_h$ is the angle between $\vec n$ and $\vec h$
- $\theta_d$ is the angle between $\vec l$ and $\vec h$ or (by symmetry) the angle between $\vec v$ and $\vec h$

Furthermore, the following vectors are useful for some specular effects:
- $\vec X$ is the tanget of the surface
- $\vec Y$ is the bitangent/binormal of the surface

# Rendering Equation
The [rendering equation](https://en.wikipedia.org/wiki/Rendering_equation), introduced in 1986 by [David Immel et al.](http://www0.cs.ucl.ac.uk/research/vr/Projects/VLF/vlfpapers/multi-pass_hybrid/Immel_D_S__A_Radiosity_Method_for_Non-Diffuse_Environments.pdf) and [James Kajiya](https://www.cse.chalmers.se/edu/year/2011/course/TDA361/2007/rend_eq.pdf), is the basis for physically based rendering. In general, the modern rendering equation (omitting time) is of the form:

$$L_o(x,\vec l,\lambda) = L_e(x,\vec l,\lambda) + L_r(x,\vec l,\lambda)$$
$$L_r(x,\vec l,\lambda) = \int_{\Omega}f_r(x,\vec v,\vec l,\lambda)L_i(x,\vec v,\lambda)(\vec v\cdot \vec n)d\vec v$$

Where $L_e$ is the emitted light and $L_r$ is the reflected light.

## $L_e$ : Emitted Light
For out model, we use a simple additive constant for the material emission. Many surfaces do not have emissive properties, and for those that do, the simple addition is more than sufficient.

## $L_r$ : Reflected Light
There are three terms that contribute to the reflected light the weakening factor, the bidirectional reflectance distribution function (BRDF), and the spectral radiance. We will examine each of these individually. 

## Weakening Factor
The weakening factor reduces the irradices of a surface at grazing angles. It is this factor alone that creates the Lambertian Diffuse model. This model takes into account the observation that surfaces exhibit full reflectance when the surface normal is point directly at the light ($\theta_i=0$) and no reflectance when the the light is at or below the surface ($\theta_i \ge 90$). Conviently, a function exists that gives $1$ when $\theta=0$ and $0$ when $\theta=90$, namely the $cos$ function. Thus, the weakening factor is commonly expressed as:

$$cos\theta_i$$

Where $\theta_i$ is the angle between the surface normal and the light direction. Since we have the normal and direction vectors as unit vectors, $cos\theta_i$ is equivalent to the dot product between the two vectors, since their magnitude's are $1$. Thus our weakening factor is expressed as:

$$\vec\omega_i\cdot \vec n$$

## Principled BRDF
In line with the Disney model, the Basilisk model will follow the mircofacet model. In general, the mircofacet model is of the form:

$$f_r(x,\vec v,\vec l,\lambda) = f_d + \frac{D(\theta _h)F(\theta _d)G(\theta _l, \theta _v)}{4 cos\theta_lcos\theta_v}$$

Where $f_d$ is the diffuse term and the rest is the specular term. Generally, anything we do in the diffuse term is a modification of the stardard lambertain diffuse, since we will inevitably multiply by $\vec\omega_i\cdot \vec n$ due to the weakening factor. According to Burley:

> "For the specular term, D is the microfacet distribution function and is responsible for the shape of the specular peak, F is the Fresnel reflection coefficient, and G is the geometric attenuation or shadowing factor."

In our model, we will be dividing by $4 cos\theta_lcos\theta_v$ because it is what the modern liturature most commonly agrees upon.

### Diffuse
As stated, much of the diffuse lobe will simply be a modification of the lambertian diffuse. The general goal of the Disney Principled diffuse model is to change the behaviour of the material when viewed at grazing angles. Additionally, we provide a secondary diffuse lobe which approximates subsurface scattering. We will now break down each material paramter that contributes to the diffuse term.

#### Base Color
Probably the simplest parameter is the base color. This is simply a 3 component vector that is multiplied to the diffuse result to give it the correct color. Basilisk allows for this to be a constant value for the material, or to be sampled from a texture.

#### Roughness
The roughness paramter changes the amount of light diffused by a material at grazing angles. Smoother materials will diffuse less light at grazing angles, and rougher materials will diffuse more light at graxing angles. We will also use the roughness parameter later on in specular term of the BRDF. To achive the desired result, we will be using an approximation of the fresnel effect, the [Schlick Fresnel](https://en.wikipedia.org/wiki/Schlick%27s_approximation), which interpolates (non-linearly) between the base reflectance ($F_{0}$) and max reflectance ($F_{90}$) fresnel reflectance a surface exhibits:

$$F_{Schlick}(\theta) = F_{0} + (F_{90} - F_{0})(1 - cos\theta)^5$$

The max reflectance is calculated using the roughness value like so:

$$F_{d90} = 0.5 + 2\cdot roughness\cdot cos^2\theta_d$$

We will assume that the base reflectance is simply $1$. The diffuse is constructed with two fresnel terms like so:

$$f_D = \frac{baseColor}{\pi}(1 + (F_{d90} - 1)(1 - cos\theta_l)^5)(1 + (F_{d90} - 1)(1 - cos\theta_v)^5)$$

If we assume that the interpolation is linear we would get the following result:

$$f_D = \frac{baseColor}{\pi}lerp(1, F_{d90}, (1 - cos\theta_l)^5)lerp(1, F_{d90}, (1 - cos\theta_v)^5)$$

However, in order to perserve physical plausibility, we did not choose to assume linearity.

#### Sheen and Sheen Tint
The sheen is an additive lobe to the diffuse term that behave much like a fresnel factor, adding extra reflectance to the material at grazing angles. The sheen tint paramter specifies how much the sheen should be tinted toward the base color. The value of the sheen is given by:

$$f_{Sheen} = sheen\cdot(1 - cos\theta_d)^5\cdot sheenTint$$

#### Subsurface Scattering
The subsurface parameter blends between the base diffuse $F_D$ calculated above and an approximation of the [Hanrahan-Krueger subsurface model](https://cseweb.ucsd.edu/~ravir/6998/papers/p165-hanrahan.pdf). For the blending, we simply use lerp between the two values according to the subsurface parameter value. We calculate the approximation like so:

$$F_{ss90} = roughness\cdot cos^2\theta_d$$

$$F_{SS} = \frac{baseColor}{\pi}(1 + (F_{ss90} - 1)(1 - cos\theta_l)^5)(1 + (F_{ss90} - 1)(1 - cos\theta_v)^5)$$

$$f_{ss} = 1.25\cdot (F_{ss}\cdot (\frac{1}{cos\theta_l + cos\theta_v} - 0.5) + 0.5)$$

Notice that much of the approximation has terms similar to the diffuse calculated above. There are a couple noticable differences between the two. First;y, the max reflectance is lower overall, not having an additive value at all. Secondly, the final adjustment to the subsurface term flatens the value across the surface. Because of these differences, the subsurface model provides a more uniform look to the material, which is what we would expect from a surface exhibting subsurface scattering. However, this is just an approximation, and not a replacement for geniuine subsurface scattering. If we want to improve our subsurface model, we will look to Burley's 2015 paper mentioned at the start of this document.

#### Putting it Together
We are now at a point where we can calculate our entire diffuse value. Using what we have defined above, the diffuse is give by:

$$f_d = lerp(f_D, f_{ss}, subsurface) + f_{sheen}$$

That completes our discussion of the diffuse model for Basilisk engine. We will now begin discussion of the specular term

### Specular
#### Specular Distribution
For the specular distribution model, Burley choose to use the [GGX distribution](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf) function over the [Beckmann distribution](https://books.google.com/books/about/The_Scattering_of_Electromagnetic_Waves.html?id=QBEIAQAAIAAJ) because of its longer tail and ability to match experimental data. The GGX distribution, also known as the [Throwbridge-Reitz distribution](https://pharr.org/matt/blog/images/average-irregularity-representation-of-a-rough-surface-for-ray-reflection.pdf)  (TR), is of the form:

$$D_{GGX} = D_{TR} = c\ \frac{1}{(\alpha^2 cos^2\theta_h + sin^2\theta_h)^2}$$

Where c is a scaling factor (we leave it arbitrary for now so that the function is in a more useful form) and $\alpha$ is mapped from the roughness value for intuitive artist control like so:

$$\alpha = roughness^2$$

In the GGX distribution, the denominator is raise to the power of two. We will call this exponent $\gamma$. For arbitrary values of $\gamma$, we call the function the Generalized Throwbridge-Reitz distribution (GTR):

$$D_{GTR} = c\ \frac{1}{(\alpha^2 cos^2\theta_h + sin^2\theta_h)^\gamma}$$

Therefore, we can view the GGX distribution as the GTR distribution with $\gamma = 2$. For $\gamma = 2$, we will choose $c=\frac{\alpha^2}{\pi}$. We can simplify the formula to the following:

$$D_{GTR_2} = \frac{\alpha^2}{\pi((\alpha^2 - 1) cos^2 \theta_h + 1)^2}$$

To add more artistic control over the specular shape, Disney makes use of the Anisotropic version of the GGX distribution function. This is the function used in Basilisk:

$$D_{GTR_{2aniso}}=\frac1\pi \frac{1}{\alpha_x\alpha_y}\frac{1}{((\frac{\vec h \cdot \vec X}{\alpha_x})^2 + (\frac{\vec h \cdot \vec Y}{\alpha_y})^2 + (\vec h \cdot \vec n)^2)^2}$$

Where $\alpha_x$ and $\alpha_y$ are given by:

$$aspect = \sqrt{1 - 0.9\ anisotropic}$$

$$\alpha_x = \frac{roughness^2}{aspect}$$

$$\alpha_y = roughness^2\cdot aspect$$

Here, anisotropic is a parameter of the material that controls the strength of the anisotropic effect, which streches the specular shape over the surface. This effect is most prominently displayed in metallic materials. 

#### Specular Fresnel Term
For specular fresnel, the Schlick Fresnel we used previously is sufficient:

$$F_{Schlick} = F_{0} + (1 - F_{0})(1 - cos\theta_d)^5$$

In this case, the base relectance $F_0$ is given by the material's specular parameter.

#### Specular Geometric Attenuation Term
For the geometric attenuation term, we will need to use a function that is derived from the $GTR_{2aniso}$ distribution function. We know from an update in 2014 that Disney actually got this function wrong in their original model. In Heitz's 2014 analysis, [Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs](https://jcgt.org/published/0003/02/03/), Heitz claims that the masking function for Burley's GTR "has yet to be found". Because of this, I will provide two different functions that could plausibly be used. First, the GGX masking used by Heitz in the previously mentioned analysis:

$$G = G_1(\vec h, \vec v)G_1(\vec h, \vec l)$$

$$G_1(\vec H, \vec S) = \frac{1}{1 + \Lambda(\vec S)}$$

$$\Lambda(\vec S) = \frac{-1 + \sqrt{1 + \frac{1}{a^2}}}{2}$$

$$a = \frac{1}{\alpha tan\theta_S}$$

The second function is taken from [Acerola's physically based model](https://github.com/GarrettGunnell/Disney-PBR/blob/main/Assets/Shaders/DisneyBRDF.shader) which is largely based on Burleys 2012 paper:

$$G_1(\vec H, \vec S) = \frac{1}{\vec n \cdot \vec S + \sqrt{\sqrt{(\vec S \cdot \vec X)\alpha_{g_x}} + \sqrt{(\vec S \cdot \vec Y)\alpha_{g_y}} + \sqrt{\vec n \cdot \vec S}}}$$
