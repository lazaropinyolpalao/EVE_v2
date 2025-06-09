#include <tinyobj.hpp>



#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>

TinyObj::TinyObj() {
	isInit_ = false;
	bigger_than_optimazable_ = false;
	cull_type_ = 1;
#ifdef RENDER_OPENGL
	virtual_buffer_object_ = -1;
	virtual_array_object_ = -1;
#endif
}

TinyObj::~TinyObj() {
	vertices_.clear();
#ifdef RENDER_OPENGL
	glDeleteBuffers(1, &virtual_buffer_object_);
	glDeleteBuffers(1, &virtual_array_object_);
	glDeleteBuffers(1, &index_buffer_object_);
#endif
#ifdef RENDER_DIRECTX11
	
#endif
}

void TinyObj::LoadObj(std::string inputfile) {
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		std::cerr << "Exited program with error code 1. " << std::endl;
		return;
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	//auto& materials = reader.GetMaterials();

	Vertex vtx;
	
	//-------------------------------
	// Load by brute force over vertices
	//-------------------------------
	

	// Loop over shapes
	//Get number of vertices to check if drawn by indexes or not, based on size of file
	unsigned int vertex_count_ = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		size_t size = shapes[s].mesh.num_face_vertices.size();
		for (size_t f = 0; f < size; f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			for (size_t v = 0; v < fv; v++) {vertex_count_++;}
		}
	}

	vertices_.reserve(vertex_count_);
	indexes_.reserve(vertex_count_);

	//Check if bigger than the optimizable loop
	bigger_than_optimazable_ = (vertex_count_ >= kVertexCountLimit);

	for (size_t s = 0; s < shapes.size(); s++) {
		vtx = { {0.0f, 0.0f, 0.0f}, // Position
					 {0.0f, 0.0f, 0.0f},  // Color
					 {0.0f, 0.0f, 0.0f},  // Normal
						{0.0f, 0.0f}				// UV
				};

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				// Vertex index
				int temp_index = idx.vertex_index *3;
				vtx.position_ = { 
					attrib.vertices[temp_index], 
					attrib.vertices[temp_index + 1], 
					attrib.vertices[temp_index + 2] 
				};

				// Optional: vertex colors
				vtx.color_ = { 
					attrib.colors[temp_index], 
					attrib.colors[temp_index + 1], 
					attrib.colors[temp_index + 2]
				};

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					temp_index = idx.normal_index * 3;
					vtx.normal_= {
						attrib.normals[temp_index],
						attrib.normals[temp_index + 1],
						attrib.normals[temp_index + 2]
					};
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					temp_index = idx.texcoord_index * 2;
					vtx.uv_ = { attrib.texcoords[temp_index] , attrib.texcoords[temp_index + 1]};
				}


				if (!bigger_than_optimazable_) {
					//*/
					//Check if it exists in the map
					bool found = false;	
					unsigned int index=-1;
					for (unsigned int i = 0; !found && i < vertices_.size(); ++i) {
						if (!found  && Vertex::Compare(vertices_[i],vtx)) {
							found = true;
							index = i;
						}
					}

					//If the vertex doesn't exist in the map, insert it
					if (!found) {
						unsigned int t= (unsigned int)vertices_.size();
						indexes_.emplace_back(t);
						vertices_.emplace_back(vtx);
					}
					//Else, grab it's index
					else {indexes_.emplace_back(index);}
				}
				else {
					indexes_.emplace_back((unsigned int)vertices_.size());
					vertices_.emplace_back(vtx);
				}
			}

			index_offset += fv;
		}
	}

	//Set the file name from the source
	char search_start = '/';
	char search_end = '.';
	size_t init_pos = -1;
	size_t end_pos = -1;

	//Find last ocurrence of the separator
	init_pos = inputfile.rfind(search_start);
	end_pos = inputfile.rfind(search_end);

	if (init_pos!= -1) {
		name_ = inputfile.substr(init_pos+1, end_pos - (init_pos+1));
	}
	else {name_ = "unnamed";}

	//Save path for further comparisons
	full_path_ = inputfile;
}

#ifdef RENDER_OPENGL
void TinyObj::InitBuffer() {
	if (vertices_.size() != 0 && !isInit_) {
		isInit_ = true;
		//Generate the virtual buffer, bind it, and allocate it
		glGenBuffers(1, &virtual_buffer_object_);
		glBindBuffer(GL_ARRAY_BUFFER, virtual_buffer_object_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices_.size(), &vertices_[0].position_.x, GL_STATIC_DRAW);

		//Generate the index buffer
		glGenBuffers(1, &index_buffer_object_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexes_.size(), &indexes_[0], GL_STATIC_DRAW);

		//Generate the vertex array
		glGenVertexArrays(1, &virtual_array_object_);
		glBindVertexArray(virtual_array_object_);

		//Enable the vertex attributes for positions, colors and normals
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1); 
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(9 * sizeof(float)));
		
		glBindVertexArray(0);
	}
}
#endif
#ifdef RENDER_DIRECTX11
void TinyObj::InitBuffer(ID3D11Device* dev, ID3D11DeviceContext* devCon) {
	if (vertices_.size() != 0 && !isInit_ ) {
		HRESULT hr;
		// Vertex Buffer
		{
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = vertices_.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * (unsigned int)vertices_.size(), D3D11_BIND_VERTEX_BUFFER);
			hr = dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, g_pVertexBuffer.GetAddressOf());
			if (FAILED(hr)) {
				printf("Missing creating vertex buffer  %#010x\n", hr);
				return ;
			}

			//Map the buffer to fill the data
			/*/
			D3D11_MAPPED_SUBRESOURCE ms;
			hr = devCon->Map(g_pVertexBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   // map the buffer
			if (FAILED(hr)) {
				printf("Missing filling vertex buffer  %#010x\n", hr);
				return;
			}
			memcpy(ms.pData, vertices_.data(), sizeof(Vertex) * vertices_.size());                // copy the data
			devCon->Unmap(g_pVertexBuffer.Get(), NULL);
			/**/
			
		}

		// Index Buffer
		{
			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = indexes_.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * (unsigned int)indexes_.size(), D3D11_BIND_INDEX_BUFFER);
			hr = dev->CreateBuffer(&indexBufferDesc, &indexBufferData, g_pIndexBuffer.GetAddressOf());

			if (FAILED(hr)) {
				printf("Missing creating index buffer  %#010x\n", hr);
				return;
			}


			/*/
			D3D11_MAPPED_SUBRESOURCE ms;
			hr = devCon->Map(g_pIndexBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   // map the buffer
			if (FAILED(hr)) {
				printf("Missing filling index buffer  %#010x\n", hr);
				return;
			}
			memcpy(ms.pData, indexes_.data(), sizeof(unsigned int) * indexes_.size());                // copy the data
			devCon->Unmap(g_pIndexBuffer.Get(), NULL);
			/**/
		}

		isInit_ = true;
	}
}
#endif
