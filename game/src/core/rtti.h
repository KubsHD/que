#pragma once


struct ClassDescriptor {
	std::string name;
};

class ClassDb {
private:
	static ClassDb* m_instance;
	std::map<std::string, ClassDescriptor*> m_database;

public:
	static ClassDb* get_instance() {
		if (m_instance == nullptr) {
			m_instance = new ClassDb();
		}
		return m_instance;
	}


};

#define RTTI_COMPONENT_NAME(NAME) \
const char* get_class_name() override { \
	return #NAME; \
} \

