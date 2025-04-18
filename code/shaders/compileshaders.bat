fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../data/shaders/lighting.sb lighting.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../data/shaders/sslr.sb sslr.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../data/shaders/ssao.sb ssao.hlsl

fxc /Tcs_5_0 /O3 /Ges /WX /EcomputeShader /Fo../../data/shaders/lighttransform.sb lighttransform.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/blitVS.sb blit.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../data/shaders/blitPS.sb blit.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/blitarrayVS.sb blitarray.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../data/shaders/blitarrayPS.sb blitarray.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/blurVS.sb blur.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderX /Fo../../data/shaders/blurXPS.sb blur.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderY /Fo../../data/shaders/blurYPS.sb blur.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/combine_tonemapVS.sb combine_tonemap.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../data/shaders/combine_tonemapPS.sb combine_tonemap.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/downsampleVS.sb downsample.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader2x2 /Fo../../data/shaders/downsample2x2PS.sb downsample.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader4x4 /Fo../../data/shaders/downsample4x4PS.sb downsample.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/gbufferVS.sb gbuffer.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderDribble /Fo../../data/shaders/gbufferDribblePS.sb gbuffer.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderRing /Fo../../data/shaders/gbufferRingPS.sb gbuffer.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/particleVS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderDrop /Fo../../data/shaders/particleDropPS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderSplash /Fo../../data/shaders/particleSplashPS.sb particle.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShaderPointLight /Fo../../data/shaders/particlePointLightPS.sb particle.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/rainbufVS.sb rainbuf.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../data/shaders/rainbufPS.sb rainbuf.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/shadowmapVS.sb shadowmap.hlsl

fxc /Tvs_5_0 /O3 /Ges /WX /EvertexShader /Fo../../data/shaders/skyboxVS.sb skybox.hlsl
fxc /Tps_5_0 /O3 /Ges /WX /EpixelShader /Fo../../data/shaders/skyboxPS.sb skybox.hlsl