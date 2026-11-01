#pragma once
#include <map>
#include <string>
#include "LateDays.h"

class Assignment {
public:
    Assignment(int assignmentId, int courseId, std::string title,
               TimePoint deadline, int maxMark);

    int assignmentId() const { return assignmentId_; }
    int courseId() const { return courseId_; }
    const std::string& title() const { return title_; }
    TimePoint deadline() const { return deadline_; }
    int maxMark() const { return maxMark_; }

    // Individual extension (FR-10). Must be later than the normal deadline.
    void addExtension(int studentId, TimePoint newDeadline);
    // The deadline that applies to this student (extended or normal)
    TimePoint deadlineFor(int studentId) const;
    bool isPastDeadline(TimePoint t, int studentId) const;

private:
    int assignmentId_;
    int courseId_;
    std::string title_;
    TimePoint deadline_;
    int maxMark_;
    std::map<int, TimePoint> extensions_;   // studentId -> new deadline
};
