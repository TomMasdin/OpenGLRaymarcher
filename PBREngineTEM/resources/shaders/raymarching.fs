#version 430 core

out vec4 fragColor;
in vec2 fragCoord;

uniform vec3 cam_pos;
uniform float iTime;
uniform float displacementMultiplier;

//Create Sphere
float distance_from_sphere(vec3 p, vec3 c, float r)
{
    return length(p - c) - r;
}

// Create Box
float sdBox(vec3 p, vec3 b, float r)
{
	vec3 d = abs(p) - b;
	return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0) - r; // Remove for partial SDF
}

float smin( float a, float b, float k )
{
    float h = max( k-abs(a-b), 0.0 )/k;
    return min( a, b ) - h*h*k*(1.0/4.0);
}

float map_the_world(vec3 p)
{
	//Animation
	float rtime = iTime;

    float time = rtime;
    time += 15.0*smoothstep(  15.0, 25.0, rtime );
    time += 20.0*smoothstep(  65.0, 80.0, rtime );
    time += 35.0*smoothstep( 105.0, 135.0, rtime );
    time += 20.0*smoothstep( 165.0, 180.0, rtime );
    time += 40.0*smoothstep( 220.0, 290.0, rtime );
    time +=  5.0*smoothstep( 320.0, 330.0, rtime );
	float frequency = sin(time);

	float displacement = (sin(displacementMultiplier * p.x)) * (sin(displacementMultiplier  * p.y)) * (sin(displacementMultiplier  * p.z)) * frequency;
    float sphere_0 = distance_from_sphere(p, vec3(0.0), 2.0);
    

    return sphere_0 + displacement;
}

vec3 calculateNormal(vec3 p)
{
	const vec3 small_step = vec3(0.001, 0.0, 0.0);
	
	float gradient_x = map_the_world(p + small_step.xyy) - map_the_world(p - small_step.xyy);
    float gradient_y = map_the_world(p + small_step.yxy) - map_the_world(p - small_step.yxy);
    float gradient_z = map_the_world(p + small_step.yyx) - map_the_world(p - small_step.yyx);
	
	vec3 normal = vec3(gradient_x, gradient_y, gradient_z);
	
	return normalize(normal);
}

vec3 ray_march(vec3 ro, vec3 rd)
{
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 255;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        float distance_to_closest = map_the_world(current_position);
		
		
        if (distance_to_closest < MINIMUM_HIT_DISTANCE) 
        {
			
            vec3 normal = calculateNormal(current_position);
			
			//Light Positions
			vec3 light_position = vec3(2.0, -5.0, 3.0);
			
			//Calculate the direction vector from intersect to light
			vec3 direction_to_light = normalize(current_position - light_position);
			
			float diffuse_intensity = max(0.0, dot(normal, direction_to_light));
			
			return vec3(1.0, 0.0, 0.0) * diffuse_intensity;
        }

        if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
        {
            break;
        }
        total_distance_traveled += distance_to_closest;
    }
    return vec3(0.0);
}

void main()
{
	vec2 uv = fragCoord;
	
	vec3 cameraPos = vec3(0.0, 0.0, -5.0);
	vec3 ro = cameraPos;
	vec3 rd = vec3(uv, 1.0);
	
	vec3 shaded_color = ray_march(ro, rd);
	
    fragColor = vec4(shaded_color , 1.0);
}