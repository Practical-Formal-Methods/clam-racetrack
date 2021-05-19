' Copyright 2019, Gurobi Optimization, LLC
'
' Solve a model with different values of the Method parameter;
' show which value gives the shortest solve time.

Imports System
Imports Gurobi

Class lpmethod_vb

    Shared Sub Main(ByVal args As String())

        If args.Length < 1 Then
            Console.WriteLine("Usage: lpmethod_vb filename")
            Return
        End If

        Try
            ' Read model and verify that it is a MIP
            Dim env As New GRBEnv()
            Dim model As New GRBModel(env, args(0))

            ' Solve the model with different values of Method
            Dim bestMethod As Integer = -1
            Dim bestTime As Double = model.get(GRB.DoubleParam.TimeLimit)
            For i As Integer = 0 To 2
                model.Reset()
                model.Parameters.Method = i
                model.Optimize()
                If model.Status = GRB.Status.OPTIMAL Then
                    bestTime = model.Runtime
                    bestMethod = i
                    ' Reduce the TimeLimit parameter to save time
                    ' with other methods
                    model.Parameters.TimeLimit = bestTime
                End If
            Next

            ' Report which method was fastest
            If bestMethod = -1 Then
                Console.WriteLine("Unable to solve this model")
            Else
                Console.WriteLine("Solved in " & bestTime & _
                                  " seconds with Method: " & bestMethod)
            End If

            ' Dispose of model and env
            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " & e.Message)
        End Try
    End Sub
End Class
