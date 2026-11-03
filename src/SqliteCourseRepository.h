#pragma once
#include "CourseRepository.h"
#include "Database.h"

class SqliteCourseRepository : public CourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    int save(const Course& course) override;
    std::optional<Course> findById(int courseId) const override;
    void enrol(int studentId, int courseId) override;
    bool isEnrolled(int studentId, int courseId) const override;
private:
    Database& db_;
};
