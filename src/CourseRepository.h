#pragma once
#include <optional>
#include "Course.h"

class CourseRepository {
public:
    virtual ~CourseRepository() = default;
    virtual int save(const Course& course) = 0;                  // insert (id 0) or update
    virtual std::optional<Course> findById(int courseId) const = 0;
    virtual void enrol(int studentId, int courseId) = 0;
    virtual bool isEnrolled(int studentId, int courseId) const = 0;
};
