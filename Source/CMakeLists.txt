set(shaderDependencies
	# Add explicit dependencies here
	Shaders/Common.glsl
)

set(shaders
	Shaders/Model.frag
	Shaders/Model.vert
	Shaders/ModelBarycentrics.geom
	Shaders/ModelIndexed.frag
	Shaders/ModelIndexed.vert
	Shaders/ModelManual.frag
	Shaders/ModelManual.vert
	Shaders/ModelNativeAMD.frag
	Shaders/ModelNativeAMD.vert
	Shaders/ModelNativeAMDTextured.frag
	Shaders/ModelPassthrough.frag
	Shaders/ModelPassthrough.geom
	Shaders/ModelPassthrough.vert
	Shaders/ModelPassthroughTextured.frag
)

set(src
	${shaders}
	BaseApplication.cpp
	BaseApplication.h
	Barycentrics.cpp
	Barycentrics.h
	DemoUtils.cpp
	DemoUtils.h
)

set(app Barycentrics)

add_executable(${app}
	${src}
)

source_group("Shaders" FILES ${shaders} ${shaderDependencies})

function(shader_compile_rule shaderName dependencies)
    add_custom_command(
        OUTPUT ${CMAKE_CFG_INTDIR}/${shaderName}.spv
        COMMAND ${GLSLC} -o ${CMAKE_CFG_INTDIR}/${shaderName}.spv ${CMAKE_CURRENT_SOURCE_DIR}/${shaderName}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${shaderName}
        DEPENDS ${dependencies}
    )
endfunction(shader_compile_rule)

foreach(shader ${shaders})
	shader_compile_rule(${shader} "${shaderDependencies}")
endforeach()

target_compile_definitions(${app} PRIVATE
	RUSH_USING_NAMESPACE # Automatically use Rush namespace
)

target_link_libraries(${app}
	meshoptimizer
	Rush
	stb
	tiny_obj_loader
)
