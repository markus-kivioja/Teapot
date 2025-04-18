fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../assets/shaders/lighting.sb lighting.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../assets/shaders/sslr.sb sslr.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../assets/shaders/ssao.sb ssao.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../assets/shaders/lighttransform.sb lighttransform.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/blitVS.sb blit.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../assets/shaders/blitPS.sb blit.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/blitarrayVS.sb blitarray.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../assets/shaders/blitarrayPS.sb blitarray.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/blurVS.sb blur.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderX /Fo../../assets/shaders/blurXPS.sb blur.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderY /Fo../../assets/shaders/blurYPS.sb blur.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/combine_tonemapVS.sb combine_tonemap.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../assets/shaders/combine_tonemapPS.sb combine_tonemap.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/downsampleVS.sb downsample.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader2x2 /Fo../../assets/shaders/downsample2x2PS.sb downsample.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader4x4 /Fo../../assets/shaders/downsample4x4PS.sb downsample.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/gbufferVS.sb gbuffer.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderDribble /Fo../../assets/shaders/gbufferDribblePS.sb gbuffer.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderRing /Fo../../assets/shaders/gbufferRingPS.sb gbuffer.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/particleVS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderDrop /Fo../../assets/shaders/particleDropPS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderSplash /Fo../../assets/shaders/particleSplashPS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderPointLight /Fo../../assets/shaders/particlePointLightPS.sb particle.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/rainbufVS.sb rainbuf.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../assets/shaders/rainbufPS.sb rainbuf.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/shadowmapVS.sb shadowmap.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../assets/shaders/skyboxVS.sb skybox.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../assets/shaders/skyboxPS.sb skybox.hlsl