#include "Assignment.h"
#include "Exceptions.h"

Assignment::Assignment(int assignmentId, int courseId, std::string title,
                       TimePoint deadline, int maxMark)
    : assignmentId_(assignmentId), courseId_(courseId), title_(std::move(title)),
      deadline_(deadline), maxMark_(maxMark) {
    if (title_.empty()) throw InvalidInputException("Assignment title cannot be empty");
    if (maxMark_ <= 0)  throw InvalidInputException("Maximum mark must be above 0");
}

void Assignment::addExtension(int studentId, TimePoint newDeadline) {
    if (newDeadline <= deadline_)
        throw InvalidInputException("An extension must be later than the deadline");
    extensions_[studentId] = newDeadline;
}

TimePoint Assignment::deadlineFor(int studentId) const {
    auto it = extensions_.find(studentId);
    return it == extensions_.end() ? deadline_ : it->second;
}

bool Assignment::isPastDeadline(TimePoint t, int studentId) const {
    return t > deadlineFor(studentId);
}
