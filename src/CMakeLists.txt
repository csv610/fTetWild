set(SOURCES
	AABBWrapper.h
	AABBWrapper.cpp


	auto_table.hpp
	auto_table.cpp

	CSGTreeParser.hpp
	CSGTreeParser.cpp

	LocalOperations.h
	LocalOperations.cpp
	EdgeSplitting.h
	EdgeSplitting.cpp
	EdgeCollapsing.h
	EdgeCollapsing.cpp
	EdgeSwapping.h
	EdgeSwapping.cpp
	VertexSmoothing.h
	VertexSmoothing.cpp

	MeshImprovement.h
	MeshImprovement.cpp
	MeshFiltering.h
	MeshFiltering.cpp
	MeshSizing.h
	MeshSizing.cpp
	MeshSurface.h
	MeshSurface.cpp


	FloatTetwild.h
	FloatTetwild.cpp

	FloatTetDelaunay.h
	FloatTetDelaunay.cpp
		TriangleInsertion.cpp
		TriangleInsertion.h

		CutMesh.cpp
		CutMesh.h

	intersections.h
	intersections.cpp

	Simplification.h
	Simplification.cpp
	SimplificationHelpers.h
	SimplificationHelpers.cpp
	MeshCleanup.h
	MeshCleanup.cpp
	EdgeCollapseSimplify.h
	EdgeCollapseSimplify.cpp
	EdgeSwapSimplify.h
	EdgeSwapSimplify.cpp

	Mesh.cpp
	Mesh.hpp

	MeshIO.hpp
	MeshIO.cpp

	Logger.hpp
	Logger.cpp

	Parameters.h

	Types.hpp

	Statistics.h

	Error.h
	MeshState.h
)

prepend_current_path(SOURCES)
float_tetwild_copy_headers(${SOURCES})
float_tetwild_set_source_group(${SOURCES})
target_sources(FloatTetwild PRIVATE ${SOURCES})

################################################################################
# Subfolders
################################################################################

add_subdirectory(external)
