#include "SubmissionService.h"
#include "Exceptions.h"

SubmissionService::SubmissionService(AssignmentRepository& assignments, CourseRepository& courses,
                                     SubmissionRepository& submissions)
    : assignments_(assignments), courses_(courses), submissions_(submissions) {}

Submission SubmissionService::submit(int studentId, int assignmentId, const std::string& content,
                                     TimePoint now) {
    if (content.empty()) throw InvalidInputException("Submission cannot be empty");

    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw InvalidInputException("Assignment not found");
    if (!courses_.isEnrolled(studentId, assignment->courseId()))
        throw AccessDeniedException("You are not enrolled in this course");

    // The student's own deadline: the extension if there is one, otherwise the normal deadline
    const TimePoint deadline = assignment->deadlineFor(studentId);

    auto existing = submissions_.findByAssignment(assignmentId, studentId);
    int id;
    if (existing) {
        existing->resubmit(content, now, deadline);      // refused after the deadline
        id = submissions_.save(*existing);
    } else {
        id = submissions_.save(Submission::createNew(assignmentId, studentId, content, now, deadline));
    }
    return *submissions_.findById(id);
}
