/******************************************************************************
 * @file DrawScene.cpp
 * @author Jay Sharma
 * @brief Code snippet containing a function that uses OpenGL to render a 
 *  single frame from a 3D scene, complete with BRDF lighting, depth pass, 
 *  shadow pass, reflection pass, and image based lighting.
 * @version 0.1
 * @date 2021-7-23
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/
void Scene::DrawScene()
{
    // Set the viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    CHECKERROR;
    // Calculate the light's position from lightSpin, lightTilt, lightDist
    lightPos = glm::vec3(lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
                         lightDist*sin(lightSpin*rad)*sin(lightTilt*rad), 
                         lightDist*cos(lightTilt*rad));

    // Update position of any continuously animating objects
    double atime = 360.0*glfwGetTime()/36;
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2, atime);

    BuildTransforms();

    // The lighting algorithm needs the inverse of the WorldView matrix
    WorldInverse = glm::inverse(WorldView);

    CHECKERROR;
    int loc, programId;

    // SHADOW
    
    // Use shadow shader and bind the FBO
    shadowProgram->Use();
    programId = shadowProgram->programId;
    fbo->Bind();

    // Set the viewport to the FBO size, clear screen
    glViewport(0, 0, fbo->width, fbo->height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    // Build Matrices from light's POV
    ViewMatrix = LookAt(lightPos, -lightPos, glm::vec3(0,0,1));
    ProjectionMatrix = Perspective(40/lightDist
                        , 40/lightDist
                        , front
                        , (mode==0) ? 1000 : back);

    // Create transformations from the light's POV and send to the shader 
    loc = glGetUniformLocation(programId, "ViewMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ViewMatrix));
    loc = glGetUniformLocation(programId, "ProjectionMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ProjectionMatrix));
    CHECKERROR;

    // Draw all objects (This recursively traverses the object hierarchy.)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    objectRoot->Draw(shadowProgram, Identity, true);
    glDisable(GL_CULL_FACE);
    CHECKERROR; 
    
    // Unbind the FBO,
    // Turn off the shader
    fbo->Unbind();
    shadowProgram->Unuse();
    CHECKERROR;

    // REFLECTION

    // Pass 1 - Top FBO (+c)
    lightingProgram->Use();
    programId = lightingProgram->programId;
    topFBO->Bind();
    glViewport(0, 0, topFBO->width, topFBO->height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECKERROR;

    const glm::mat4 B = Translate(0.5f, 0.5f, 0.5f) * Scale(0.5f, 0.5f, 0.5f);
    ShadowMatrix = B * ProjectionMatrix * ViewMatrix;
    CHECKERROR;

    // The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp

    // Textures
    glActiveTexture(GL_TEXTURE2);   // Activate texture unit 2
    glBindTexture(GL_TEXTURE_2D, fbo->textureID);   // Load texture into it
    loc = glGetUniformLocation(programId, "shadowMap");
    glUniform1i(loc, 2);

    // IRRADIANCE
    glActiveTexture(GL_TEXTURE5);   // Activate texture unit 5
    glBindTexture(GL_TEXTURE_2D, IRRTexture->textureId);   // Load texture into it
    loc = glGetUniformLocation(programId, "IRR");
    glUniform1i(loc, 5);

    // For computing Irradiance Map
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGB, 400, 200, 0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);

    // Reflection FBOs
    glActiveTexture(GL_TEXTURE10);   // Activate texture unit 3
    glBindTexture(GL_TEXTURE_2D, topFBO->textureID);   // Load texture into it
    loc = glGetUniformLocation(programId, "topRefl");
    glUniform1i(loc, 10);   

    loc = glGetUniformLocation(programId, "ShadowMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ShadowMatrix));
    CHECKERROR;

    // Draw
    objectRoot->Draw(lightingProgram, Identity, false);
    topFBO->Unbind();

    // Pass 2 - Bottom FBO (-c)
    botFBO->Bind();
    glViewport(0, 0, botFBO->width, botFBO->height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE11);   // Activate texture unit 4
    glBindTexture(GL_TEXTURE_2D, botFBO->textureID);   // Load texture into it
    loc = glGetUniformLocation(programId, "botRefl");
    glUniform1i(loc, 11);
    CHECKERROR;

    objectRoot->Draw(lightingProgram, Identity, false);
    botFBO->Unbind();

    ////////////////////////////////////////////////////////////////////////////////
    // Lighting pass - Render from Eye's POV to Screen
    ////////////////////////////////////////////////////////////////////////////////

    // Set the viewport, and clear the screen
    glViewport(0, 0, width, height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);
    CHECKERROR;

    // Draw all objects (This recursively traverses the object hierarchy.)
    objectRoot->Draw(lightingProgram, Identity, true);
    CHECKERROR; 
    
    lightingProgram->Unuse();

    ////////////////////////////////////////////////////////////////////////////////
    // End of Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
}
