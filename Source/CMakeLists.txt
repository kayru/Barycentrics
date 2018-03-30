set(shaderDependencies
	# Add explicit dependencies here
	Shaders/Common.glsl
)

set(shaders
	Shaders/Model.vert
	Shaders/Model.frag
)

set(src
	${shaders}
	BaseApplication.cpp
	BaseApplication.h
	Barycentrics.cpp
	Barycentrics.h
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
	Rush
	stb
	tiny_obj_loader
)
